#include "BytecodeBuilder.h"
#include <format>
#include <iostream>
using namespace std;



int ConstTable::AddOrNot(const Token& tok)
{
	auto found = _constMap.find(tok);
	if(found != _constMap.end())
		return found->second;

	_constMap[tok] = (uint16_t)_constMap.size();
	return (int)_constMap.size() - 1;
}

int ConstTable::GetIdx(const Token& tok) const
{
	auto found = _constMap.find(tok);
	return found == _constMap.end() ? -1 : (int)found->second;
}


SymbolTable::SymbolTable()
{
	_scopeTbl.push_back(SCOPE_FUNC);
	_symTbl.resize(1);
}

SymbolTable::~SymbolTable()
{}

SymbolTable::SymbolData SymbolTable::GetSymbolData(const string& name) const
{
	int firstFunc = -1;
	for(int i=(int)_symTbl.size()-1; i>=0; i--)
	{
		auto found = _symTbl[i].find( { .name = name } );
		if(found != _symTbl[i].end())
		{
			if(firstFunc >= 0)
			{
				return SymbolData{ .idx = { .kind = Idx::GLOBAL, .idx = found->second }, .sym = found->first };
			}
			else
				return SymbolData{ .idx = { .kind = Idx::LOCAL, .idx = found->second - GetBehindFuncScopeCnt(i) }, .sym = found->first };
		}

		if(_scopeTbl[i] == SCOPE_FUNC)
		{
			if(firstFunc < 0)
				firstFunc = i;
			else
			{//TODO
				//throw 'n';
			}
		}
	}

	return SymbolData();
}

int SymbolTable::GetNewSlotIdx() const
{
	size_t sz = 0;
	for(auto& scope : _symTbl)
	{
		sz += scope.size();
	}
	return (int)sz;
}

int SymbolTable::GetGlobalSymbolCnt() const
{
	if(_symTbl.size() <= 1)
		return 0;

	size_t sz = 0;
	for(int i=0; i<_symTbl.size()-1; i++)
	{
		sz += _symTbl[i].size();
	}
	return (int)sz;
}

int SymbolTable::GetLocalSymbolCnt() const
{
	return (int)_symTbl.back().size();
}

int SymbolTable::GetSymbolCnt() const
{
	return GetGlobalSymbolCnt() + GetLocalSymbolCnt();
}

int SymbolTable::GetBehindFuncScopeCnt(int idx) const
{
	if(idx <= 0) return 0;
	if(idx >= _symTbl.size()) return -1;

	if(_symTbl.size() <= 1)
		return 0;

	size_t sz = 0;
	for(int i=idx; i>=0; i--)
	{
		if(_scopeTbl[i] == SCOPE_FUNC)
		{
			for(int j=i-1; j>=0; j--)
			{
				sz += _symTbl[j].size();
			}
			break;
		}
	}
	return (int)sz;
}


void SymbolTable::AddBlockScope()
{
	_symTbl.resize(_symTbl.size() + 1);
	_scopeTbl.push_back(SCOPE_BLOCK);
}

void SymbolTable::AddFuncScope()
{
	_symTbl.resize(_symTbl.size() + 1);
	_scopeTbl.push_back(SCOPE_FUNC);
}

void SymbolTable::PopScope()
{
	_symTbl.pop_back();
	_scopeTbl.pop_back();
}

SymbolTable::Idx SymbolTable::AddOrNot(const Symbol& sym)
{
	auto idx = GetIdx(sym.name);
	if(idx.kind != Idx::NONE)
	{
		return idx;
	}

	int newIdx = GetNewSlotIdx();
	_symTbl.back()[sym] = newIdx;

	return Idx{ .kind = Idx::LOCAL, .idx = newIdx - GetBehindFuncScopeCnt((int)_symTbl.size()-1) };
}

SymbolTable::Idx SymbolTable::GetIdx(const string& name) const
{
	return GetSymbolData(name).idx;
}

Symbol SymbolTable::GetSymbol(const string& name) const
{
	return GetSymbolData(name).sym;
}


BytecodeBuilder::BytecodeBuilder(const TreeNode& code)
	: _code(code)
	, _reg(0)
{
}

BytecodeBuilder::~BytecodeBuilder()
{
}

bool BytecodeBuilder::Build(Bytecode& retCode)
{
	for(const auto& stmt : _code.childs)
	{
		BuildStmt(*stmt);
	}


	for(int i=0; i<_bytecode.size(); i++)
	{
		cout << format("{:4} {}\n", i+1, _bytecode[i].codeStr);
	}

	map<int, Token> sorted;
	for(auto& c : _constTbl._constMap)
	{
		sorted[ c.second ] = c.first;
	}
	for(auto& [idx, tok] : sorted)
	{
		Constant c;
		if(tok == EToken::Num)
		{//todo float
			c.type = Constant::NUM;
			c.num = stoll(tok.val);
		}
		else
		{
			c.type = Constant::STR;
			c.str = tok.val;
		}

		retCode._consts.push_back(c);
	}

	retCode._code = _bytecode;

	return true;
}

void BytecodeBuilder::BuildBlockOpen()
{
	_bytecode.push_back( {  .kind = EOpcode::PushSp, .codeStr = "pushsp" } );
	if(!_loopStack.empty()) _loopStack.top().pushSpCnt++;
	_symTbl.AddBlockScope();
}

void BytecodeBuilder::BuildBlockClose()
{
	_symTbl.PopScope();
	if(!_loopStack.empty()) _loopStack.top().pushSpCnt--;
	_bytecode.push_back( {  .kind = EOpcode::PopSp, .codeStr = "popsp" } );
}

bool BytecodeBuilder::BuildStmt(const TreeNode& stmt)
{
	switch(stmt.self.kind)
	{
	case EToken::For : return BuildFor(stmt);
	case EToken::If : return BuildIf(stmt);
	case EToken::Fn : return BuildFn(stmt);
	case EToken::LBrace : return BuildCompound(stmt);
	case EToken::Return : return BuildReturn(stmt);
	case EToken::Continue : return BuildContinue(stmt);
	case EToken::Break : return BuildBreak(stmt);
	default: ;
	}
	return BuildExp(stmt, true);
}

bool BytecodeBuilder::BuildExp(const TreeNode& stmt, bool root)
{
	uint32_t regStack = _reg;
	Inst::Assign inst;

	if(stmt.self == EToken::Id || stmt.self.IsLiteral())
	{
		inst.dstKind = (uint8_t)ERefKind::Reg;
		inst.dst = _reg;

		int srcNum = -1;
		if(stmt.self.IsLiteral())
		{
			inst.src1Kind = (uint8_t)ERefKind::Const;
			inst.src1 = _constTbl.AddOrNot(stmt.self);
		}
		else
		{
			auto idx = _symTbl.AddOrNot({ .name = stmt.self.val, .kind = ESymbol::Var });

			inst.src1Kind = idx.kind == SymbolTable::Idx::LOCAL ? (uint8_t)ERefKind::LocalVar : (uint8_t)ERefKind::GlobalVar;
			inst.src1 = (uint16_t)idx.idx;
		}

		string rhs = format("{}({}{})", stmt.self.val, inst.src1Kind == (uint8_t)ERefKind::LocalVar ? 'l' : 'g', inst.src1);

		_bytecode.push_back( { .kind = EOpcode::Assign, .codeStr = format("{} = {}", format("t{}", _reg), rhs ) } );
		_bytecode.back().code.resize(sizeof(inst));
		memcpy(_bytecode.back().code.data(), &inst, sizeof(inst));
		return true;
	}

	if(stmt.self == EToken::Invoke)
	{
		for(size_t i = 1; i<stmt.childs.size(); i++)
		{
			if(!BuildExp(*stmt.childs[i], false))
				throw 'n';
			_reg++;
		}

		_reg = regStack;

		_bytecode.push_back( { .codeStr = format("t{} = invoke {}", _reg, _symTbl.GetSymbol(stmt.childs[0]->self.val).pos) } );
		Inst::Invoke ivk{ .pos = (uint32_t)_symTbl.GetSymbol(stmt.childs[0]->self.val).pos, .numPrms = (uint32_t)stmt.childs.size()-1 };
		_bytecode.back().kind = EOpcode::Invoke;
		_bytecode.back().code.resize(sizeof(ivk));
		memcpy(_bytecode.back().code.data(), &ivk, sizeof(ivk));
		return true;
	}


	TreeNode* lhs = !stmt.childs.empty() ? stmt.childs.front() : nullptr;
	TreeNode* rhs = stmt.childs.size() > 1 ? stmt.childs.back() : nullptr;


	if(!lhs)
	{
		throw 'n';
	}

	string lhsStr;
	string rhsStr;
	string codeStr;

	if(lhs->self != EToken::Id && !lhs->self.IsLiteral())
	{
		if(!BuildExp(*lhs, false))
		{
			throw 'n';
		}
		lhsStr = format("t{}", _reg);
		inst.src1Kind = (uint8_t)ERefKind::Reg;
		inst.src1 = (uint16_t)_reg;
		_reg++;
	}
	else
	{
		if(lhs->self.IsLiteral())
		{
			inst.src1Kind = (uint8_t)ERefKind::Const;
			inst.src1 = _constTbl.AddOrNot(lhs->self);
			lhsStr = lhs->self.val;
		}
		else
		{
			auto idx = _symTbl.AddOrNot({ .name = lhs->self.val, .kind = ESymbol::Var });

			inst.src1Kind = idx.kind == SymbolTable::Idx::LOCAL ? (uint8_t)ERefKind::LocalVar : (uint8_t)ERefKind::GlobalVar;
			inst.src1 = (uint16_t)idx.idx;

			lhsStr = format("{}({}{})", lhs->self.val, inst.src1Kind == (uint8_t)ERefKind::LocalVar ? 'l' : 'g', inst.src1);
		}
	}

	if(rhs)
	{
		if(rhs->self != EToken::Id && !rhs->self.IsLiteral())
		{
			if(!BuildExp(*rhs, false))
			{
				throw 'n';
			}
			rhsStr = format("t{}", _reg);
			inst.src2Kind = (uint8_t)ERefKind::Reg;
			inst.src2 = (uint16_t)_reg;
			_reg++;
		}
		else
		{
			if(rhs->self.IsLiteral())
			{
				inst.src2Kind = (uint8_t)ERefKind::Const;
				inst.src2 = _constTbl.AddOrNot(rhs->self);
				rhsStr = rhs->self.val;
			}
			else
			{
				auto idx = _symTbl.GetIdx(rhs->self.val);
				if(idx.kind == SymbolTable::Idx::NONE)
				{
					throw 'n';
				}

				inst.src2Kind = idx.kind == SymbolTable::Idx::LOCAL ? (uint8_t)ERefKind::LocalVar : (uint8_t)ERefKind::GlobalVar;
				inst.src2 = (uint16_t)idx.idx;

				rhsStr = format("{}({}{})", rhs->self.val, inst.src2Kind == (uint8_t)ERefKind::LocalVar ? 'l' : 'g', inst.src2);
			}


		}

		codeStr = format("{} {} {}", lhsStr, stmt.self.val, rhsStr);
		inst.op = (uint8_t)stmt.self.kind;
	}
	else
	{
		if(stmt.self.IsPrefixUnary())
		{
			codeStr = stmt.self.val + lhsStr;
			inst.op = (uint8_t)stmt.self.kind;
			inst.src2Kind = inst.src1Kind;
			inst.src2 = inst.src1;
			inst.src1Kind = (uint8_t)ERefKind::None;
			inst.src1 = 0;
		}
		else if(stmt.self == EToken::LParen)
		{
			codeStr = lhsStr;
			inst.op = (uint8_t)EToken::None;
		}
		else
		{
			throw 'n';
		}
	}

	_reg = regStack;
	if(!root)
	{
		inst.dstKind = (uint8_t)ERefKind::Reg;
		inst.dst = _reg;
		codeStr = format("{} = {}", format("t{}", _reg), codeStr);
	}
	else
	{
		inst.dstKind = (uint8_t)EToken::None;
		inst.dst = 0;
	}

	_bytecode.push_back( { .codeStr = codeStr } );
	_bytecode.back().code.resize(sizeof(inst));
	memcpy(_bytecode.back().code.data(), &inst, sizeof(inst));
	return true;
}

bool BytecodeBuilder::BuildReturn(const TreeNode& stmt)
{
	if(stmt.childs.empty())
	{//TODO
	}
	else
	{
		if(!BuildExp(*stmt.childs[0], true))
		{
			throw 'n';
		}
	}
	return true;
}

bool BytecodeBuilder::BuildContinue(const TreeNode& stmt)
{
	if(stmt.self != EToken::Continue)
		throw 'n';

	for(int i=0; i<_loopStack.top().pushSpCnt; i++)
	{
		_bytecode.push_back( { .kind = EOpcode::PopSp, .codeStr = "popsp" } );
	}
	_bytecode.push_back( { .kind = EOpcode::Jmp, } );
	_loopStack.top().contLines.push_back((int)_bytecode.size()-1);
	return true;
}

bool BytecodeBuilder::BuildBreak(const TreeNode& stmt)
{
	if(stmt.self != EToken::Break)
		throw 'n';

	for(int i=0; i<_loopStack.top().pushSpCnt; i++)
	{
		_bytecode.push_back( { .kind = EOpcode::PopSp, .codeStr = "popsp" } );
	}
	_bytecode.push_back( { .kind = EOpcode::Jmp, } );
	_loopStack.top().breakLines.push_back((int)_bytecode.size()-1);
	return true;
}

bool BytecodeBuilder::BuildFor(const TreeNode& stmt)
{
	if(stmt.self != EToken::For)
		throw 'n';

	auto& init = *stmt.childs[0];
	auto& cond = *stmt.childs[1];
	auto& update = *stmt.childs[2];
	auto& block = *stmt.childs[3];

	string condStr;

	BuildExp(init, true);

	size_t loopStart = _bytecode.size() + 1;
	BuildExp(cond, false);
	condStr = format("jz {}, ", format("t{}", _reg));
	_bytecode.push_back( { .codeStr = condStr } );
	size_t condLine = _bytecode.size() - 1;
	Inst::Jz jz;
	jz.testKind = (uint8_t)ERefKind::Reg;
	jz.test = _reg;

	_loopStack.push({});

	//TODO single stmt
	BuildCompound(block);

	size_t loopEnd = _bytecode.size() + 1;
	BuildExp(update, true);

	Inst::Jmp jmp{ .pos = (uint32_t)loopStart };
	Instruction inst;
	inst.kind = EOpcode::Jmp;
	inst.code.resize(sizeof(jmp));
	*(Inst::Jmp*)inst.code.data() = jmp;
	inst.codeStr = format("jmp {}", loopStart);
	_bytecode.push_back(inst);

	size_t updateEnd = _bytecode.size() + 1;

	_bytecode[condLine].codeStr += to_string(updateEnd);
	_bytecode[condLine].kind = EOpcode::Jz;
	jz.pos = (uint32_t)updateEnd;
	_bytecode[condLine].code.resize(sizeof(jz));
	*(Inst::Jz*)_bytecode[condLine].code.data() = jz;

	for(auto& cl : _loopStack.top().contLines)
	{
		auto& inst = _bytecode[cl];
		inst.kind = EOpcode::Jmp;
		Inst::Jmp jmp{ .pos = (uint32_t)loopEnd };
		inst.code.resize(sizeof(jmp));
		*(Inst::Jmp*)inst.code.data() = jmp;
		inst.codeStr = format("jmp {}", loopEnd);
	}

	for(auto& bl : _loopStack.top().breakLines)
	{
		auto& inst = _bytecode[bl];
		inst.kind = EOpcode::Jmp;
		Inst::Jmp jmp{ .pos = (uint32_t)updateEnd };
		inst.code.resize(sizeof(jmp));
		*(Inst::Jmp*)inst.code.data() = jmp;
		inst.codeStr = format("jmp {}", updateEnd);
	}

	_loopStack.pop();

	return true;
}

bool BytecodeBuilder::BuildIf(const TreeNode& stmt)
{
	if(stmt.self != EToken::If)
		throw 'n';

	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	string condStr;

	BuildExp(test, false);
	condStr = format("jz {}, ", format("t{}", _reg));
	_bytecode.push_back( { .codeStr = condStr } );
	size_t condLine = _bytecode.size() - 1;
	Inst::Jz jz;
	jz.testKind = (uint8_t)ERefKind::Reg;
	jz.test = _reg;

	BuildStmt(_true);
	_bytecode.push_back( { .codeStr = format("jmp ") } );
	size_t skipLine = _bytecode.size() - 1;

	_bytecode[condLine].codeStr += to_string(_bytecode.size() + 1);
	_bytecode[condLine].kind = EOpcode::Jz;
	jz.pos = (uint32_t)_bytecode.size() + 1;
	_bytecode[condLine].code.resize(sizeof(jz));
	*(Inst::Jz*)_bytecode[condLine].code.data() = jz;

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!BuildStmt(_false))
		{
			throw 'n';
		}
	}

	_bytecode[skipLine].codeStr += to_string(_bytecode.size() + 1);
	_bytecode[skipLine].kind = EOpcode::Jmp;
	Inst::Jmp jmp{ .pos = (uint32_t)_bytecode.size() + 1 };
	_bytecode[skipLine].code.resize(sizeof(jmp));
	*(Inst::Jmp*)_bytecode[skipLine].code.data() = jmp;

	return true;
}

bool BytecodeBuilder::BuildFn(const TreeNode& stmt)
{
	uint32_t regStack = _reg;
	_reg = 0;

	if(stmt.self != EToken::Fn)
		throw 'n';

	size_t skipLine = _bytecode.size();
	_bytecode.push_back( { .codeStr = "jmp " } );

	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];

	_bytecode.push_back( {  .kind = EOpcode::PushSp, .codeStr = "pushsp" } );
	_symTbl.AddFuncScope();

	Symbol sym;
	sym.name = name;
	sym.pos = _bytecode.size();
	sym.kind = ESymbol::Fn;
	for(auto& p : params)
	{
		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);

		auto idx = _symTbl.AddOrNot( { .name = p->self.val, .kind = ESymbol::Var } );
		_bytecode.push_back( { .kind = EOpcode::Assign, .codeStr = format("{} = t{}", prm.name, _reg) } );
		Inst::Assign as;
		as.dstKind = (uint8_t)ERefKind::LocalVar;
		as.dst = idx.idx;
		as.src1Kind = (uint8_t)ERefKind::Reg;
		as.src1 = _reg++;
		_bytecode.back().code.resize(sizeof(as));
		memcpy(_bytecode.back().code.data(), &as, sizeof(as));
	}

	_reg = 0;
	if(block.self == EToken::LBrace)
	{
		if(!BuildCompound(block))
		{
			throw 'n';
		}
	}
	else if(!BuildStmt(block))
	{
		throw 'n';
	}

	_reg = regStack;

	BuildBlockClose();
	_bytecode.push_back( { .kind = EOpcode::Ret, .codeStr = "ret" } );
	_symTbl.AddOrNot(sym);

	_bytecode[skipLine].codeStr += to_string(_bytecode.size()+1);
	Inst::Jmp jmp{ .pos = (uint32_t)_bytecode.size() + 1 };
	_bytecode[skipLine].kind = EOpcode::Jmp;
	_bytecode[skipLine].code.resize(sizeof(jmp));
	memcpy(_bytecode[skipLine].code.data(), &jmp, sizeof(jmp));
	return true;
}

bool BytecodeBuilder::BuildCompound(const TreeNode& stmt)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	BuildBlockOpen();

	for(auto& itm : stmt.childs)
	{
		BuildStmt(*itm);
	}

	BuildBlockClose();
	return true;
}
