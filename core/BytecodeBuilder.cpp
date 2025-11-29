#include "BytecodeBuilder.h"
#include "BuiltinFuncTable.h"
#include <format>
#include <iostream>
using namespace std;


static const BuiltinFuncTable _builtinFuncTbl;


inline ERefKind ToRefKind(SymbolTable::Idx::Kind idxKind)
{
	switch(idxKind)
	{
	case SymbolTable::Idx::LOCAL: return ERefKind::LocalVar;
	case SymbolTable::Idx::GLOBAL: return ERefKind::GlobalVar;
	case SymbolTable::Idx::FIELD: return ERefKind::FieldVar;
	}
	return ERefKind::None;
}

#define TO_REF_KIND_U8(__idxKind__) ((uint8_t)ToRefKind(__idxKind__))


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
			if(found->first.kind == ESymbol::Field)
			{
				return SymbolData{ .idx = { .kind = Idx::FIELD, .idx = found->second }, .sym = found->first };
			}
			else
			{
				if(firstFunc >= 0)
				{
					return SymbolData{ .idx = { .kind = Idx::GLOBAL, .idx = found->second }, .sym = found->first };
				}
				else
					return SymbolData{ .idx = { .kind = Idx::LOCAL, .idx = found->second - GetBehindFuncScopeCnt(i) }, .sym = found->first };
			}
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

SymbolTable::Idx SymbolTable::AddOrNot(const Symbol& sym, int wantIdx /* = -1 */)
{
	auto idx = GetIdx(sym.name);
	if(idx.kind != Idx::NONE)
	{
		return idx;
	}

	int newIdx = wantIdx < 0 ?GetNewSlotIdx() : wantIdx;
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


BytecodeBuilder::BytecodeBuilder()
	: _reg(0)
{
	_prg._mainCode.PushBytecode<EOpcode::Noop>();
}

BytecodeBuilder::~BytecodeBuilder()
{
}


bool BytecodeBuilder::Build(const TreeNode& code, Program& retProgram)
{
	for(const auto& stmt : code.childs)
	{
		if(!BuildStmt(_prg._mainCode, *stmt))
			return false;
	}

	auto main = _symTbl.GetSymbol("main");
	if(main.kind == ESymbol::Fn)
	{
		Op::Call cal{ (uint16_t)main.params.size(), 0, (uint32_t)main.pos,  };
		_prg._mainCode.PushBytecode(cal, code.self.line);
	}

	_prg._consts.resize(_constTbl._constMap.size());
	for(auto& [tok, idx] : _constTbl._constMap)
	{
		Constant c;
		if(tok == EToken::Int)
		{
			c._type = Constant::INT;
			c._int = stoll(tok.val, nullptr, 0);
		}
		else if(tok == EToken::Float)
		{
			c._type = Constant::FLOAT;
			c._float = stod(tok.val);
		}
		else
		{
			c._type = Constant::STR;
			c._str = tok.val;
		}

		_prg._consts[idx] = c;
	}

	retProgram = _prg;
	return true;
}

void BytecodeBuilder::BuildBlockOpen(Bytecode& retCtx)
{
	retCtx.PushBytecode<EOpcode::PushSp>();
	if(!_loopStack.empty()) _loopStack.top().pushSpCnt++;
	if(!_fnStack.empty()) _fnStack.top().pushSpCnt++;
	_symTbl.AddBlockScope();
}

void BytecodeBuilder::BuildBlockClose(Bytecode& retCtx)
{
	_symTbl.PopScope();
	if(!_loopStack.empty()) _loopStack.top().pushSpCnt--;
	if(!_fnStack.empty()) _fnStack.top().pushSpCnt--;
	retCtx.PushBytecode<EOpcode::PopSp>();
}

bool BytecodeBuilder::BuildStmt(Bytecode& retCtx, const TreeNode& stmt)
{
	switch(stmt.self.kind)
	{
	case EToken::Include : return BuildInclude(retCtx, stmt);
	case EToken::For : return BuildFor(retCtx, stmt);
	case EToken::If : return BuildIf(retCtx, stmt);
	case EToken::Fn : return BuildFn(retCtx, stmt);
	case EToken::LBrace : return BuildCompound(retCtx, stmt);
	case EToken::Return : return BuildReturn(retCtx, stmt);
	case EToken::Continue : return BuildContinue(retCtx, stmt);
	case EToken::Break : return BuildBreak(retCtx, stmt);
	case EToken::Class : return BuildClass(retCtx, stmt);
	default: ;
	}
	return BuildExp(retCtx, stmt, true);
}

bool BytecodeBuilder::BuildInclude(Bytecode& retCtx, const TreeNode& stmt)
{
	auto& incName = *stmt.childs[0];

	_symTbl.AddOrNot({ incName.self.val, ESymbol::Mod });

	int idx = _constTbl.AddOrNot(incName.self);
	Op::Inc inc { .inc = (uint16_t)idx };
	retCtx.PushBytecode(inc, stmt.self.line);

	//TODO
	_prg._moduleTable[ incName.self.val ] = idx;
	return true;
}

bool BytecodeBuilder::BuildExp(Bytecode& retCtx, const TreeNode& stmt, bool root)
{
	uint32_t regStack = _reg;
	Op::Assign inst;

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

			inst.src1Kind = TO_REF_KIND_U8(idx.kind);
			inst.src1 = (uint16_t)idx.idx;
		}

		retCtx.PushBytecode(inst, stmt.self.line);
		return true;
	}

	if(stmt.self == EToken::Invoke)
	{
		const Token& ivkType = stmt.childs[0]->self;

		if(ivkType == EToken::Dot)
		{	//TODO generalize
			if(!BuildExp(retCtx, *stmt.childs[0], false))
				throw 'n';
			_reg++;
		}

		for(size_t i = 1; i<stmt.childs.size(); i++)
		{
			if(!BuildExp(retCtx, *stmt.childs[i], false))
				throw 'n';
			_reg++;
		}
		_reg = regStack;

		if (ivkType == EToken::LParen)
		{//todo dynamic eval
		}

		if(ivkType == EToken::Dot)
		{	//TODO generalize
			Op::Invoke ivk{ .dstKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg, .numArgs = (uint8_t)(stmt.childs.size()-1) };
			retCtx.PushBytecode(ivk, stmt.self.line);
			return true;
		}

		auto builtinFuncId = _builtinFuncTbl.GetFuncId(ivkType.val);
		if(builtinFuncId)
		{
			Op::Call cal{ (uint16_t)(stmt.childs.size()-1), 0, builtinFuncId };
			retCtx.PushBytecode(cal, stmt.self.line);
			return true;
		}

		int constIdx = _constTbl.GetIdx(ivkType);
		if(constIdx >= 0)
		{
			if(_prg._classTable.contains(ivkType.val))
			{//TODO
				Op::NewCls nc{ .dstKind = (uint8_t)ERefKind::Const, .dst = (uint16_t)constIdx, .numArgs = (uint8_t)(stmt.childs.size()-1) };
				retCtx.PushBytecode(nc, stmt.self.line);
			}
			else if(_prg._moduleTable.contains(ivkType.val))
			{//TODO
				Op::NewMod nm{ .dstKind = (uint8_t)ERefKind::Const, .dst = (uint16_t)constIdx, .numArgs = (uint8_t)(stmt.childs.size()-1) };
				retCtx.PushBytecode(nm, stmt.self.line);
			}
			else
			{
				Op::Invoke ivk{ .dstKind = (uint8_t)ERefKind::Const, .dst = (uint16_t)constIdx, .numArgs = (uint8_t)(stmt.childs.size()-1) };
				retCtx.PushBytecode(ivk, stmt.self.line);
			}
		}
		else
		{
			Op::Call cal{ (uint16_t)(stmt.childs.size()-1), 0, (uint32_t)_symTbl.GetSymbol(stmt.childs[0]->self.val).pos  };
			retCtx.PushBytecode(cal, stmt.self.line);
		}
		return true;
	}

	if(stmt.self == EToken::List)
	{
		return BuildList(retCtx, stmt);
	}

	if(stmt.self == EToken::Dict)
	{
		return BuildDict(retCtx, stmt);
	}


	if(stmt.self == EToken::Index || stmt.self == EToken::LValueIndex)
	{
		return BuildIndex(retCtx, stmt);
	}


	TreeNode* lhs = !stmt.childs.empty() ? stmt.childs.front().get() : nullptr;
	TreeNode* rhs = stmt.childs.size() > 1 ? stmt.childs.back().get() : nullptr;


	if(!lhs)
	{
		throw 'n';
	}

	if(lhs->self != EToken::Id && !lhs->self.IsLiteral())
	{
		if(!BuildExp(retCtx, *lhs, false))
		{
			throw 'n';
		}
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
		}
		else
		{
			auto idx = _symTbl.AddOrNot({ .name = lhs->self.val, .kind = ESymbol::Var });

			inst.src1Kind = TO_REF_KIND_U8(idx.kind);
			inst.src1 = (uint16_t)idx.idx;
		}
	}


	int logicalOpLine = -1;
	if(stmt.self == EToken::And || stmt.self == EToken::Or)
	{
		logicalOpLine = retCtx.nextCodeSlot();
		retCtx.PushBytecode<EOpcode::Noop>();
	}

	if(rhs)
	{
		if(rhs->self != EToken::Id && !rhs->self.IsLiteral())
		{
			if(!BuildExp(retCtx, *rhs, false))
			{
				throw 'n';
			}
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
			}
			else
			{
				auto idx = _symTbl.GetIdx(rhs->self.val);
				if(idx.kind == SymbolTable::Idx::NONE)
				{
					throw 'n';
				}

				inst.src2Kind = TO_REF_KIND_U8(idx.kind);
				inst.src2 = (uint16_t)idx.idx;
			}
		}

		inst.op = (uint8_t)stmt.self.kind;
	}
	else
	{
		if(stmt.self.IsIncDecOp())
		{
			uint8_t srcKind;
			uint16_t srcIdx;
			auto& LValue = stmt.childs.front()->self;
			switch(stmt.childs.front()->self.kind)
			{
			case EToken::Id:
				{
					auto idx = _symTbl.GetIdx(LValue.val);
					srcKind = TO_REF_KIND_U8(idx.kind);
					srcIdx = (uint16_t)idx.idx;
					break;
				}

			case EToken::Index:
			case EToken::LBracket:
				{
					auto idxInst = retCtx._code.back();
					if(idxInst != EOpcode::Index)//TODO
						throw 'n';

					const Op::Index& idx = *((Op::Index*)idxInst.code.data());
					Op::LValueIndex lidx = { idx.dstKind, idx.idxKind, idx.dst, idx.idx };
					retCtx.FillBytecode(retCtx.endOfCode(), lidx, stmt.self.line);

					srcKind = inst.src1Kind;
					srcIdx = inst.src1;
					break;
				}

			case EToken::Dot: //TODO
			default:
				throw 'n';
			}

			inst.op = (uint8_t)stmt.self.kind;
			if(stmt.self == EToken::PreInc || stmt.self == EToken::PreDec)
			{
				inst.src1Kind = (uint8_t)ERefKind::None;
				inst.src1 = 0;
				inst.src2Kind = srcKind;
				inst.src2 = srcIdx;
			}
			else
			{
				inst.src1Kind =srcKind;
				inst.src1 = srcIdx;
				inst.src2Kind = (uint8_t)ERefKind::None;
				inst.src2 = 0;
			}
		}
		else if(stmt.self.IsPrefixUnary())
		{
			inst.op = (uint8_t)stmt.self.kind;
			inst.src2Kind = inst.src1Kind;
			inst.src2 = inst.src1;
			inst.src1Kind = (uint8_t)ERefKind::None;
			inst.src1 = 0;
		}
		else if(stmt.self == EToken::LParen)
		{
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
	}
	else
	{
		inst.dstKind = (uint8_t)EToken::None;
		inst.dst = 0;
	}

	if(stmt.self == EToken::And || stmt.self == EToken::Or)
	{
		Op::Assign as;
		as.dstKind = inst.dstKind;
		as.dst = inst.dst;
		as.src1Kind = inst.src2Kind;
		as.src1 = inst.src2;
		retCtx.PushBytecode(as, stmt.self.line);

		if(stmt.self == EToken::And)
		{
			Op::Jz jz;
			jz.testKind = (uint8_t)inst.src1Kind;
			jz.test = inst.src1;
			jz.pos = retCtx.nextCodeSlot();
			retCtx.FillBytecode(logicalOpLine, jz, stmt.self.line);
		}
		else
		{
			Op::Jnz jnz;
			jnz.testKind = (uint8_t)inst.src1Kind;
			jnz.test = inst.src1;
			jnz.pos = retCtx.nextCodeSlot();
			retCtx.FillBytecode(logicalOpLine, jnz, stmt.self.line);
		}
	}
	else
	{
		retCtx.PushBytecode(inst, stmt.self.line);
	}
	return true;
}

bool BytecodeBuilder::BuildReturn(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.childs.empty())
	{//TODO
	}
	else
	{
		if(!BuildExp(retCtx, *stmt.childs[0], false))
		{
			throw 'n';
		}

		for(int i=0; i<_fnStack.top().pushSpCnt; i++)
		{
			retCtx.PushBytecode<EOpcode::PopSp>();
		}
		retCtx.PushBytecode<EOpcode::Ret>();
	}
	return true;
}

bool BytecodeBuilder::BuildContinue(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::Continue)
		throw 'n';

	for(int i=0; i<_loopStack.top().pushSpCnt; i++)
	{
		retCtx.PushBytecode<EOpcode::PopSp>();
	}
	retCtx.PushBytecode<EOpcode::Noop>();
	_loopStack.top().contLines.push_back(retCtx.endOfCode());
	return true;
}

bool BytecodeBuilder::BuildBreak(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::Break)
		throw 'n';

	for(int i=0; i<_loopStack.top().pushSpCnt; i++)
	{
		retCtx.PushBytecode<EOpcode::PopSp>();
	}
	retCtx.PushBytecode<EOpcode::Noop>();
	_loopStack.top().breakLines.push_back(retCtx.endOfCode());
	return true;
}

bool BytecodeBuilder::BuildList(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::List)
		throw 'n';

	uint32_t regStack = _reg;

	_reg++;
	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
			throw 'n';
		_reg++;
	}

	_reg = regStack;

	Op::ListSet ls{ .dstKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg };
	retCtx.PushBytecode(ls, stmt.self.line);

	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		regStack++;
		Op::ListAdd la{ .dstKind = (uint8_t)ERefKind::Reg, .srcKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg, .src = (uint16_t)regStack };
		retCtx.PushBytecode(la, stmt.self.line);
	}
	return true;
}

bool BytecodeBuilder::BuildDict(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::Dict)
		throw 'n';

	uint32_t regStack = _reg;

	_reg++;
	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
			throw 'n';
		_reg++;
		if(!BuildExp(retCtx, *stmt.childs[i]->childs[0], false))
			throw 'n';
		_reg++;
	}

	_reg = regStack;

	Op::DictSet ds{ .dstKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg };
	retCtx.PushBytecode(ds, stmt.self.line);

	regStack++;
	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		Op::DictAdd da{
			.dstKind = (uint8_t)ERefKind::Reg,
			.keyKind = (uint8_t)ERefKind::Reg,
			.valKind = (uint8_t)ERefKind::Reg,
			.dst = (uint16_t)_reg,
			.key = (uint16_t)regStack++,
			.val = (uint16_t)regStack++,
		};
		retCtx.PushBytecode(da, stmt.self.line);
	}
	return true;
}

bool BytecodeBuilder::BuildIndex(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::Index && stmt.self != EToken::LValueIndex)
		throw 'n';

	uint32_t regStack = _reg;

	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
			throw 'n';
		_reg++;
	}

	_reg = regStack;

	if(stmt.self == EToken::Index)
	{
		Op::Index li{ .dstKind = (uint8_t)ERefKind::Reg, .idxKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg, .idx = (uint16_t)(_reg+1) };
		retCtx.PushBytecode(li, stmt.self.line);
	}
	else
	{
		Op::LValueIndex lli{ .dstKind = (uint8_t)ERefKind::Reg, .idxKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg, .idx = (uint16_t)(_reg+1) };
		retCtx.PushBytecode(lli, stmt.self.line);
	}

	return true;
}

bool BytecodeBuilder::BuildClass(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::Class)
		throw 'n';

	Class cls;
	cls.name = stmt.self.val;

	int fieldidx = 0;
	for(auto& substmt : stmt.childs)
	{
		if(substmt->self == EToken::Assign)
		{
			Symbol sym;
			sym.name = substmt->childs.front()->self.val;
			sym.kind = ESymbol::Field;
			_symTbl.AddOrNot(sym, fieldidx++);

			cls._fields.push_back(sym);

			if(!BuildExp(cls._initer, *substmt, true))
			{
				return false;
			}
		}
		else
		{//fn
			auto insert = cls._funcTable.insert({substmt->self.val, {}});
			if(!BuildFn(insert.first->second, *substmt))
			{
				return false;
			}
		}
	}

	Token id = stmt.self;
	id.kind = EToken::Id;
	int idx = _constTbl.AddOrNot(id);
	_prg._classTable[ cls.name ] = cls;
	return true;
}

bool BytecodeBuilder::BuildFor(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::For)
		throw 'n';

	auto& init = *stmt.childs[0];
	auto& cond = *stmt.childs[1];
	auto& update = *stmt.childs[2];
	auto& block = *stmt.childs[3];

	BuildExp(retCtx, init, true);

	size_t loopStart = retCtx.nextCodeSlot();
	BuildExp(retCtx, cond, false);

	size_t condLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	Op::Jz jz;
	jz.testKind = (uint8_t)ERefKind::Reg;
	jz.test = _reg;

	_loopStack.push({});

	BuildStmt(retCtx, block);

	size_t loopEnd = retCtx.nextCodeSlot();
	BuildExp(retCtx, update, true);

	Op::Jmp jmp{ .pos = (uint32_t)loopStart };
	retCtx.PushBytecode(jmp, stmt.self.line);

	size_t updateEnd = retCtx.nextCodeSlot();
	jz.pos = (uint32_t)updateEnd;
	retCtx.FillBytecode((int)condLine, jz, stmt.self.line);

	for(auto& cl : _loopStack.top().contLines)
	{
		Op::Jmp jmp{ .pos = (uint32_t)loopEnd };
		retCtx.FillBytecode(cl, jmp, stmt.self.line);
	}

	for(auto& bl : _loopStack.top().breakLines)
	{
		Op::Jmp jmp{ .pos = (uint32_t)updateEnd };
		retCtx.FillBytecode(bl, jmp, stmt.self.line);
	}

	_loopStack.pop();

	return true;
}

bool BytecodeBuilder::BuildIf(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::If)
		throw 'n';

	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	BuildExp(retCtx, test, false);

	size_t condLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	Op::Jz jz;
	jz.testKind = (uint8_t)ERefKind::Reg;
	jz.test = _reg;

	BuildStmt(retCtx, _true);

	size_t skipLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	jz.pos = (uint32_t)retCtx.nextCodeSlot();
	retCtx.FillBytecode((int)condLine, jz, stmt.self.line);

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!BuildStmt(retCtx, _false))
		{
			throw 'n';
		}
	}

	Op::Jmp jmp{ .pos = (uint32_t)retCtx.nextCodeSlot() };
	retCtx.FillBytecode((int)skipLine, jmp, stmt.self.line);
	return true;
}

bool BytecodeBuilder::BuildFn(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;
	_reg = 0;

	if(stmt.self != EToken::Fn)
		throw 'n';

	size_t skipLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];


	_fnStack.push(FnControl());

	//TODO make scope function
	retCtx.PushBytecode<EOpcode::PushSp>();
	_fnStack.top().pushSpCnt++;

	Symbol sym;
	sym.name = name;
	sym.pos = retCtx.endOfCode();
	sym.kind = ESymbol::Fn;
	_symTbl.AddOrNot(sym);

	_symTbl.AddFuncScope();

	for(auto& p : params)
	{
		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);

		auto idx = _symTbl.AddOrNot( { .name = p->self.val, .kind = ESymbol::Var } );
		Op::Assign as;
		as.dstKind = (uint8_t)ERefKind::LocalVar;
		as.dst = idx.idx;
		as.src1Kind = (uint8_t)ERefKind::Reg;
		as.src1 = _reg++;
		retCtx.PushBytecode(as, p->self.line);
	}

	_reg = 0;
	if(block.self == EToken::LBrace)
	{
		if(!BuildCompound(retCtx, block))
		{
			throw 'n';
		}
	}
	else if(!BuildStmt(retCtx, block))
	{
		throw 'n';
	}

	_reg = regStack;

	BuildBlockClose(retCtx);
	retCtx.PushBytecode<EOpcode::Ret>();

	Op::Jmp jmp{ .pos = (uint32_t)retCtx.nextCodeSlot() };
	retCtx.FillBytecode((int)skipLine, jmp, stmt.self.line);
	return true;
}

bool BytecodeBuilder::BuildCompound(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.self != EToken::LBrace)
		throw 'n';

	BuildBlockOpen(retCtx);

	for(auto& itm : stmt.childs)
	{
		BuildStmt(retCtx, *itm);
	}

	BuildBlockClose(retCtx);
	return true;
}
