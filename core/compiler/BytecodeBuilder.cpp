#include "BytecodeBuilder.h"
#include "BuiltinFuncTable.h"
#include "NamespaceUtil.h"
#include <stdexcept>
#include <filesystem>
#include <iostream>
using namespace std;


namespace ycom
{

static const BuiltinFuncTable _builtinFuncTbl;


inline ERefKind ToRefKind(ScopeManager::Idx::Kind idxKind)
{
	switch(idxKind)
	{
	case ScopeManager::Idx::LOCAL: return ERefKind::LocalVar;
	case ScopeManager::Idx::GLOBAL: return ERefKind::GlobalVar;
	case ScopeManager::Idx::FIELD: return ERefKind::FieldVar;
	}
	return ERefKind::None;
}

#define TO_REF_KIND_U8(__idxKind__) ((uint8_t)ToRefKind(__idxKind__))


int ConstTable::AddOrNot(const Token& tok)
{
	auto con = TokenToConstant(tok);
	return AddOrNot(con);
}

int ConstTable::GetIdx(const Token& tok) const
{
	auto con = TokenToConstant(tok);
	return GetIdx(con);
}

int ConstTable::AddOrNot(const Constant& con)
{
	auto found = _constMap.find(con);
	if(found != _constMap.end())
		return found->second;

	_constMap[con] = (uint16_t)_constMap.size();
	return (int)_constMap.size() - 1;
}

int ConstTable::GetIdx(const Constant& con) const
{
	auto found = _constMap.find(con);
	return found == _constMap.end() ? -1 : (int)found->second;
}

Constant ConstTable::TokenToConstant(const Token& tok) const
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
	return c;
}


BytecodeBuilder::BytecodeBuilder()
	: _reg(0)
{
	_prg._mainCode.PushBytecode<EOpcode::Noop>();
}
BytecodeBuilder::~BytecodeBuilder()
{
}
bool BytecodeBuilder::Build(const TreeNode& code, Program& retProgram, const std::unordered_map<std::string, Program>* programTable /* = nullptr */, const std::vector<std::string>& paths /* = */ )
{
	_prgTbl = programTable;
	//TODO commonize with Semantic...
	_paths = paths;
	if(_paths.empty())
		_paths.push_back(filesystem::current_path().string());

	for(const auto& stmt : code.childs)
	{
		if(!BuildStmt(_prg._mainCode, *stmt))
			return false;
	}

	auto main = _scopeMgr.GetSymbol("main");
	if(main.kind == ESymbol::Fn)
	{
		Op::Call cal;
		cal.numPrms = (uint8_t)main.params.size();
		cal.dstKind = (uint8_t)ERefKind::Reg;
		cal.dst = _reg;
		cal.seg = 0;
		cal.pos = (uint32_t)main.pos;
		_prg._mainCode.PushBytecode(cal, code.self.line);
	}

	_prg._consts.resize(_constTbl._constMap.size());
	for(auto& [con, idx] : _constTbl._constMap)
	{
		_prg._consts[idx] = con;
	}

	for(auto& [_, v] : _scopeMgr.GetGlobalSymbolTable())
	{//global
		auto& s = v.sym;
		auto& i = v.idx;
		switch(v.sym.kind)
		{
		case ESymbol::Var: _prg._globalTable[s.name] = GlobalSymbol{ .kind = EGlobalSymbol::Var, .name = s.name, .idx = (uint32_t)i.idx }; break;
		case ESymbol::Fn: _prg._globalTable[s.name] = GlobalSymbol{ .kind = EGlobalSymbol::Fn, .name = s.name, .pos = (uint32_t)s.pos, .prms = (uint32_t)s.params.size() }; break;
		}
	}
	for(auto& [k, v] : _prg._classTable)
	{
		_prg._globalTable[k] = GlobalSymbol{ .kind = EGlobalSymbol::Cls, .name = k, .cls = v };
	}

	retProgram = _prg;
	return true;
}


ScopeManager::Idx BytecodeBuilder::GetNamespacePathIdx() const
{
	return _scopeMgr.GetIdx(_nsCtx.Get());
}


void BytecodeBuilder::BuildBlockOpen(Bytecode& retCtx)
{
	retCtx.PushBytecode<EOpcode::PushSp>();
	if(!_loopStack.empty()) _loopStack.top().pushSpCnt++;
	if(!_fnStack.empty()) _fnStack.top().pushSpCnt++;
	_scopeMgr.AddLocalScope();
}

void BytecodeBuilder::BuildBlockClose(Bytecode& retCtx)
{
	_scopeMgr.PopScope();
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
	const auto& incStr = incName.self.val;

	NamespaceUtil::Resolution res;
	for(auto& base : _paths)
	{
		res = NamespaceUtil::ResolveInclude(incStr, base);
		//TODO qaz module
		if(filesystem::exists(res.absPath + ".y"))
			break;
		res = {};
	}

	//TODOqaz function... resolve module
	if(_prgTbl && _prgTbl->contains(res.absPath))
	{
		//TODOqaz namespace map update except real path
		_nsTracker.AddTrackingPath(res.namespacePath);

		//TODO
		auto idx = _scopeMgr.AddOrNot({ res.namespacePath, ESymbol::Prg });

		Token tokInc = stmt.childs[0]->self;
		tokInc.val = res.absPath;
		int nameIdx = _constTbl.AddOrNot(tokInc);
		Op::Inc inc { .dstKind = TO_REF_KIND_U8(idx.kind), .dst = (uint16_t)idx.idx, .inc = (uint16_t)nameIdx };
		retCtx.PushBytecode(inc, stmt.self.line);
	}
	else
	{
		//TODO qaz
		auto idx = _scopeMgr.AddOrNot({ incName.self.val, ESymbol::Mod });

		int modNameIdx = _constTbl.AddOrNot(incName.self);
		Op::Inc inc { .dstKind = TO_REF_KIND_U8(idx.kind), .dst = (uint16_t)idx.idx, .inc = (uint16_t)modNameIdx };
		retCtx.PushBytecode(inc, stmt.self.line);

		_prg._moduleTable[ incName.self.val ] = modNameIdx;
	}

	return true;
}

bool BytecodeBuilder::BuildReturn(Bytecode& retCtx, const TreeNode& stmt)
{
	if(stmt.childs.empty())
	{//TODO to handle no return value
		for(int i=0; i<_fnStack.top().pushSpCnt; i++)
		{
			retCtx.PushBytecode<EOpcode::PopSp>();
		}
		retCtx.PushBytecode<EOpcode::Ret>();
	}
	else
	{
		if(!BuildExp(retCtx, *stmt.childs[0], false))
		{//TODO log
			return false;
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
	for(int i=0; i<_loopStack.top().pushSpCnt; i++)
	{
		retCtx.PushBytecode<EOpcode::PopSp>();
	}
	retCtx.PushBytecode<EOpcode::Noop>();
	_loopStack.top().breakLines.push_back(retCtx.endOfCode());
	return true;
}

bool BytecodeBuilder::BuildClass(Bytecode& retCtx, const TreeNode& stmt)
{
	Class& cls = _prg._classTable[ stmt.self.val ];
	cls.name = stmt.self.val;

	Token id = stmt.self;
	id.kind = EToken::Id;
	int idx = _constTbl.AddOrNot(id);

	_clsStack.push(&cls);
	_scopeMgr.AddClassScope();

	for(auto& substmt : stmt.childs)
	{
		if(substmt->self == EToken::Assign)
		{
			if(!BuildExp(cls._initer, *substmt, true))
			{//TODO cleanup
				return false;
			}
		}
	}

	for(auto& substmt : stmt.childs)
	{
		if(substmt->self.val == cls.name)
		{//ctor - noop
		}
		else if(substmt->self == EToken::Fn)
		{//fn
			cls._funcMap[ substmt->self.val ] = cls._funcs.size();
			cls._funcs.push_back({});
		}
	}

	for(auto& substmt : stmt.childs)
	{
		if(substmt->self.val == cls.name)
		{//ctor
			if(!BuildFnReal(cls._ctor, *substmt))
			{//TODO cleanup
				return false;
			}
		}
		else if(substmt->self == EToken::Fn)
		{//fn
			if(!BuildFnReal(cls._funcs[cls._funcMap[ substmt->self.val ]], *substmt))
			{//TODO cleanup
				return false;
			}
		}
	}

	_scopeMgr.PopScope();
	_clsStack.pop();
	return true;
}

bool BytecodeBuilder::BuildFor(Bytecode& retCtx, const TreeNode& stmt)
{
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
	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];

	if(!BuildExp(retCtx, test, false))
	{//TODO log
		return false;
	}

	size_t condLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	Op::Jz jz;
	jz.testKind = (uint8_t)ERefKind::Reg;
	jz.test = _reg;

	if(!BuildStmt(retCtx, _true))
	{//TODO log
		return false;
	}

	size_t skipLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	jz.pos = (uint32_t)retCtx.nextCodeSlot();
	retCtx.FillBytecode((int)condLine, jz, stmt.self.line);

	if(stmt.childs.size() > 2)
	{
		auto& _false = *stmt.childs[2];
		if(!BuildStmt(retCtx, _false))
		{//TODO log
			return false;
		}
	}

	Op::Jmp jmp{ .pos = (uint32_t)retCtx.nextCodeSlot() };
	retCtx.FillBytecode((int)skipLine, jmp, stmt.self.line);
	return true;
}

bool BytecodeBuilder::BuildCompound(Bytecode& retCtx, const TreeNode& stmt)
{
	BuildBlockOpen(retCtx);

	for(auto& itm : stmt.childs)
	{
		BuildStmt(retCtx, *itm);
	}

	BuildBlockClose(retCtx);
	return true;
}


bool BytecodeBuilder::BuildInvokeExp(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;
	const Token& ivkType = stmt.childs[0]->self;

	{//TODO qaz
		int constIdx = _constTbl.GetIdx(ivkType);
		if(constIdx < 0)
		{
			if(_clsStack.empty() || !_clsStack.top()->_funcMap.contains(stmt.childs.front()->self.val))
			{
				auto sym = _scopeMgr.GetSymbol(stmt.childs[0]->self.val);
				if(sym != ESymbol::None && sym != ESymbol::Fn)
				{
					_reg++;
				}
			}
		}
	}

	if(ivkType == EToken::Dot)
	{	//TODO generalize
		if(!BuildExp(retCtx, *stmt.childs[0], false))
		{//TODO log
			return false;
		}
		_reg++;
	}

	for(size_t i = 1; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
		{//TODO log
			return false;
		}
		_reg++;
	}
	_reg = regStack;

	if (ivkType == EToken::LParen)
	{//todo dynamic eval
		throw std::logic_error("not implemented");
	}

	if(ivkType == EToken::Dot)
	{	//TODO generalize
		Op::Invoke ivk{ .numArgs = (uint8_t)(stmt.childs.size()-1), .dstKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg };
		retCtx.PushBytecode(ivk, stmt.self.line);
		return true;
	}

	auto builtinFuncId = _builtinFuncTbl.GetFuncId(ivkType.val);
	if(builtinFuncId)
	{
		Op::Call cal;
		cal.numPrms = (uint8_t)(stmt.childs.size()-1);
		cal.dstKind = (uint8_t)ERefKind::Reg;
		cal.dst = _reg;
		cal.seg = 0;
		cal.pos = builtinFuncId;
		retCtx.PushBytecode(cal, stmt.self.line);
		return true;
	}

	int constIdx = _constTbl.GetIdx(ivkType);
	if(constIdx >= 0)
	{
		if(_prg._classTable.contains(ivkType.val))
		{//TODO
			Op::NewCls nc{ .numArgs = (uint16_t)(stmt.childs.size()-1), .nameKind = (uint8_t)ERefKind::Const, .dstKind = (uint8_t)ERefKind::Reg, .name = (uint16_t)constIdx, .dst = (uint16_t)_reg };
			retCtx.PushBytecode(nc, stmt.self.line);
		}
		else if(_prg._moduleTable.contains(ivkType.val))
		{//TODO
			Op::NewMod nm{ .numArgs = (uint16_t)(stmt.childs.size()-1), .nameKind = (uint8_t)ERefKind::Const, .dstKind = (uint8_t)ERefKind::Reg, .name = (uint16_t)constIdx, .dst = (uint16_t)_reg };
			retCtx.PushBytecode(nm, stmt.self.line);
		}
		else
		{//TODO
			return false;
		}
	}
	else
	{
		if(!_clsStack.empty() && _clsStack.top()->_funcMap.contains(stmt.childs.front()->self.val))
		{
			auto& fmap = _clsStack.top()->_funcMap;
			uint16_t idx = (uint16_t)fmap[stmt.childs.front()->self.val];

			Op::Invoke ivk{ .numArgs = (uint8_t)(stmt.childs.size()-1), .dstKind = (uint8_t)ERefKind::MemberFunc, .dst = (uint16_t)idx };
			retCtx.PushBytecode(ivk, stmt.self.line);
		}
		else
		{
			auto sym = _scopeMgr.GetSymbol(stmt.childs[0]->self.val);
			if(sym == ESymbol::None)
			{//TODO qaz
				throw 'n';
			}
			else if(sym == ESymbol::Fn)
			{
				Op::Call cal;
				cal.numPrms = (uint8_t)(stmt.childs.size()-1);
				cal.dstKind = (uint8_t)ERefKind::Reg;
				cal.dst = _reg;
				cal.seg = 0;
				cal.pos = (uint32_t)sym.pos;
				retCtx.PushBytecode(cal, stmt.self.line);
			}
			else
			{//TODO optimize
				auto idx = _scopeMgr.GetIdx(stmt.childs[0]->self.val);

				Op::Assign as{ .dstKind = (uint8_t)ERefKind::Reg, .src1Kind = TO_REF_KIND_U8(idx.kind), .dst = (uint16_t)_reg, .src1 = (uint16_t)idx.idx };
				retCtx.PushBytecode(as, stmt.self.line);

				Op::Invoke ivk{ .numArgs = (uint8_t)(stmt.childs.size()-1), .dstKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg };
				retCtx.PushBytecode(ivk, stmt.self.line);
			}
		}
	}
	return true;
}

bool BytecodeBuilder::BuildListExp(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;

	_reg++;
	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
		{//TODO log
			return false;
		}
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

bool BytecodeBuilder::BuildDictExp(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;

	_reg++;
	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
		{//TODO log
			return false;
		}
		_reg++;
		if(!BuildExp(retCtx, *stmt.childs[i]->childs[0], false))
		{//TODO log
			return false;
		}
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

bool BytecodeBuilder::BuildBytesExp(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;

	if(!stmt.childs.empty())
	{
		_reg++;
		if(!BuildExp(retCtx, *stmt.childs[0], false))
		{//TODO log
			return false;
		}
	}
	_reg = regStack;

	Op::BytesSet bs{ .dstKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg };
	if(!stmt.childs.empty())
	{
		bs.szKind = (uint8_t)ERefKind::Reg;
		bs.sz = (uint16_t)(_reg+1);
	}
	retCtx.PushBytecode(bs, stmt.self.line);
	return true;
}

bool BytecodeBuilder::BuildIndexExp(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;

	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
		{//TODO log
			return false;
		}
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

bool BytecodeBuilder::BuildLValueFieldExp(Bytecode& retCtx, const TreeNode& stmt)
{
	uint32_t regStack = _reg;

	for(size_t i = 0; i<stmt.childs.size(); i++)
	{
		if(!BuildExp(retCtx, *stmt.childs[i], false))
		{//TODO log
			return false;
		}
		_reg++;
	}

	_reg = regStack;

	Op::LValueField lvf{ .dstKind = (uint8_t)ERefKind::Reg, .fieldKind = (uint8_t)ERefKind::Reg, .dst = (uint16_t)_reg, .field = (uint16_t)(_reg+1) };
	retCtx.PushBytecode(lvf, stmt.self.line);
	return true;
}

bool BytecodeBuilder::BuildTernaryExp(Bytecode& retCtx, const TreeNode& stmt)
{
	auto& test = *stmt.childs[0];
	auto& _true = *stmt.childs[1];
	auto& _false = *stmt.childs[2];

	if(!BuildExp(retCtx, test, false))
	{//TODO log
		return false;
	}

	size_t condLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	Op::Jz jz;
	jz.testKind = (uint8_t)ERefKind::Reg;
	jz.test = _reg;

	if(!BuildExp(retCtx, _true, false))
	{//TODO log
		return false;
	}

	size_t skipLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	jz.pos = (uint32_t)retCtx.nextCodeSlot();
	retCtx.FillBytecode((int)condLine, jz, stmt.self.line);

	if(!BuildExp(retCtx, _false, false))
	{//TODO log
		return false;
	}

	Op::Jmp jmp{ .pos = (uint32_t)retCtx.nextCodeSlot() };
	retCtx.FillBytecode((int)skipLine, jmp, stmt.self.line);
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
		{	//TODO make table
			switch(stmt.self.kind)
			{
			case EToken::Null: inst.src1 = 0; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			case EToken::True: inst.src1 = 1; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			case EToken::False:inst.src1 = 2; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			case EToken::This: inst.src1 = 3; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			default:
				inst.src1Kind = (uint8_t)ERefKind::Const;
				inst.src1 = _constTbl.AddOrNot(stmt.self);
			}
		}
		else // qaz namespace
		{
			auto idx = _scopeMgr.AddOrNot({ .name = stmt.self.val, .kind = ESymbol::Var });

			inst.src1Kind = TO_REF_KIND_U8(idx.kind);
			inst.src1 = (uint16_t)idx.idx;
		}

		retCtx.PushBytecode(inst, stmt.self.line);
		return true;
	}

	switch(stmt.self.kind)
	{
	case EToken::Invoke: return BuildInvokeExp(retCtx, stmt);
	case EToken::List: return BuildListExp(retCtx, stmt);
	case EToken::Dict: return BuildDictExp(retCtx, stmt);
	case EToken::Bytes: return BuildBytesExp(retCtx, stmt);
	case EToken::Index:
	case EToken::LValueIndex: return BuildIndexExp(retCtx, stmt);
	case EToken::LValueField: return BuildLValueFieldExp(retCtx, stmt);
	case EToken::Question: return BuildTernaryExp(retCtx, stmt);
	}

	TreeNode* lhs = !stmt.childs.empty() ? stmt.childs.front().get() : nullptr;
	TreeNode* rhs = stmt.childs.size() > 1 ? stmt.childs.back().get() : nullptr;

	if(!lhs)
	{//TODO log
		return false;
	}

	if(lhs->self != EToken::Id && !lhs->self.IsLiteral())
	{
		if(!BuildExp(retCtx, *lhs, false))
		{//TODO log
			return false;
		}

		//qaz
		if(!_nsCtx.IsEmpty())
		{
			if(_nsTracker.IsTerminal(_nsCtx))
			{
				auto idx = GetNamespacePathIdx();
				inst.src1Kind = TO_REF_KIND_U8(idx.kind);
				inst.src1 = (uint16_t)idx.idx;
				_nsCtx.Clear();
			}
		}
		else
		{
			inst.src1Kind = (uint8_t)ERefKind::Reg;
			inst.src1 = (uint16_t)_reg;
			_reg++;
		}
	}
	else
	{
		if(lhs->self.IsLiteral())
		{
			//TODO make table
			switch(lhs->self.kind)
			{
			case EToken::Null: inst.src1 = 0; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			case EToken::True: inst.src1 = 1; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			case EToken::False:inst.src1 = 2; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			case EToken::This: inst.src1 = 3; inst.src1Kind = (uint8_t)ERefKind::Literal; break;
			default:
				inst.src1Kind = (uint8_t)ERefKind::Const;
				inst.src1 = _constTbl.AddOrNot(lhs->self);
			}
		}
		else if(lhs->childs.empty() && _nsTracker.IsExistingIfAppend(_nsCtx, lhs->self.val))
		{// qaz namespace start
			_nsCtx.Append(lhs->self.val);
			if(_nsTracker.IsTerminal(_nsCtx))
			{
				auto idx = _scopeMgr.GetIdx(lhs->self.val);
				inst.src1Kind = TO_REF_KIND_U8(idx.kind);
				inst.src1 = (uint16_t)idx.idx;
				_nsCtx.Clear();
			}
		}
		else
		{
			auto kind = ESymbol::Var;
			if(_scopeMgr.GetCurScope() == ScopeManager::SCOPE_CLASS)
			{
				auto& cls = *_clsStack.top();
				if(!cls._fieldMap.contains(lhs->self.val))
				{
					Symbol sym;
					sym.name = lhs->self.val;
					sym.kind = ESymbol::Field;
					_scopeMgr.AddOrNot(sym);

					cls._fieldMap[ sym.name ] = cls._fields.size();
					cls._fields.push_back(sym);
				}
				kind = ESymbol::Field;
			}
			auto idx = _scopeMgr.AddOrNot({ .name = lhs->self.val, .kind = kind });

			inst.src1Kind = TO_REF_KIND_U8(idx.kind);
			inst.src1 = (uint16_t)idx.idx;
		}
	}

	int logicalOpLine = -1;
	if(stmt.self == EToken::And || stmt.self == EToken::Or)
	{
		logicalOpLine = retCtx.nextCodeSlot();
		retCtx.PushBytecode<EOpcode::Noop>();
		retCtx.PushBytecode<EOpcode::Noop>();
	}

	if(rhs)
	{
		if(rhs->self != EToken::Id && !rhs->self.IsLiteral())
		{
			if(!BuildExp(retCtx, *rhs, false))
			{//TODO log
				return false;
			}
			inst.src2Kind = (uint8_t)ERefKind::Reg;
			inst.src2 = (uint16_t)_reg;
			_reg++;
		}
		else
		{
			if(stmt.self == EToken::Dot && rhs->childs.empty() && _nsTracker.IsExistingIfAppend(_nsCtx, rhs->self.val))
			{ // qaz namespacing
				_nsCtx.Append(rhs->self.val);
			}
			else if(rhs->self.IsLiteral())
			{		//TODO make table
				switch(rhs->self.kind)
				{
				case EToken::Null: inst.src2 = 0; inst.src2Kind = (uint8_t)ERefKind::Literal; break;
				case EToken::True: inst.src2 = 1; inst.src2Kind = (uint8_t)ERefKind::Literal; break;
				case EToken::False:inst.src2 = 2; inst.src2Kind = (uint8_t)ERefKind::Literal; break;
				default:
					inst.src2Kind = (uint8_t)ERefKind::Const;
					inst.src2 = _constTbl.AddOrNot(rhs->self);
				}
			}
			else
			{
				auto idx = _scopeMgr.GetIdx(rhs->self.val);
				if(idx.kind == ScopeManager::Idx::NONE)
				{//TODO log
					return false;
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
			switch(LValue.kind)
			{
			case EToken::Id:
				{
					auto idx = _scopeMgr.GetIdx(LValue.val);
					srcKind = TO_REF_KIND_U8(idx.kind);
					srcIdx = (uint16_t)idx.idx;
					break;
				}

			case EToken::LValueIndex:
			case EToken::LValueField:
				srcKind = inst.src1Kind;
				srcIdx = inst.src1;
				break;

			default://TODO log
				return false;
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
		{//TODO log
			return false;
		}
	}

	_reg = regStack;
	if(!_nsCtx.IsEmpty())
	{	//qaz namespace resolve
		if(_nsTracker.IsTerminal(_nsCtx))
		{
			auto idx = GetNamespacePathIdx();
			inst.src1Kind = TO_REF_KIND_U8(idx.kind);
			inst.src1 = (uint16_t)idx.idx;
			inst.op = (uint8_t)EToken::None;
			_nsCtx.Clear();
		}
		else
		{
			return true;
		}
	}

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
		int logopln = logicalOpLine;
		if((EToken)inst.dstKind != EToken::None)
		{
			Op::Assign as;
			as.dstKind = inst.dstKind;
			as.dst = inst.dst;
			as.src1Kind = inst.src2Kind;
			as.src1 = inst.src2;
			retCtx.PushBytecode(as, stmt.self.line);

			as.dstKind = inst.dstKind;
			as.dst = inst.dst;
			as.src1Kind = inst.src1Kind;
			as.src1 = inst.src1;
			retCtx.FillBytecode(logicalOpLine, as, stmt.self.line);
			logopln = logicalOpLine+1;
		}

		if(stmt.self == EToken::And)
		{
			Op::Jz jz;
			jz.testKind = (uint8_t)inst.src1Kind;
			jz.test = inst.src1;
			jz.pos = retCtx.nextCodeSlot();
			retCtx.FillBytecode(logopln, jz, stmt.self.line);
		}
		else
		{
			Op::Jnz jnz;
			jnz.testKind = (uint8_t)inst.src1Kind;
			jnz.test = inst.src1;
			jnz.pos = retCtx.nextCodeSlot();
			retCtx.FillBytecode(logopln, jnz, stmt.self.line);
		}
	}
	else
	{
		retCtx.PushBytecode(inst, stmt.self.line);
	}
	return true;
}

bool BytecodeBuilder::BuildFnReal(Bytecode& retCtx, const TreeNode& stmt, ESymbol fnType /* = ESymbol::Fn */)
{
	uint32_t regStack = _reg;
	_reg = 0;

	size_t skipLine = retCtx.nextCodeSlot();
	retCtx.PushBytecode<EOpcode::Noop>();

	auto& name = stmt.self.val;
	auto& params = stmt.childs[0]->childs;
	auto& block = *stmt.childs[1];

	Symbol sym;
	sym.name = name;
	sym.pos = retCtx.nextCodeSlot();
	sym.kind = fnType;
	_scopeMgr.AddOrNot(sym);

	_fnStack.push(FnControl());
	BuildBlockOpen(retCtx);

	for(auto& p : params)
	{
		Param prm;
		prm.name = p->self.val;
		sym.params.push_back(prm);

		auto idx = _scopeMgr.AddOrNot( { .name = prm.name, .kind = ESymbol::Var } );
	}

	_reg += params.size() - 1;
	for(int i=params.size() - 1; i>=0; i--)
	{
		auto& p = params[i];
		auto idx = _scopeMgr.GetIdx(p->self.val);

		Op::Assign as;
		as.dstKind = (uint8_t)ERefKind::LocalVar;
		as.dst = idx.idx;
		as.src1Kind = (uint8_t)ERefKind::Reg;
		as.src1 = _reg--;
		retCtx.PushBytecode(as, p->self.line);
	}

	_reg = 0;
	if(block.self == EToken::LBrace)
	{
		if(!BuildCompound(retCtx, block))
		{//TODO log
			return false;
		}
	}
	else if(!BuildStmt(retCtx, block))
	{//TODO log
		return false;
	}

	_reg = regStack;

	BuildBlockClose(retCtx);
	_fnStack.pop();
	retCtx.PushBytecode<EOpcode::Ret>();

	Op::Jmp jmp{ .pos = (uint32_t)retCtx.nextCodeSlot() };
	retCtx.FillBytecode((int)skipLine, jmp, stmt.self.line);
	return true;
}

bool BytecodeBuilder::BuildClosure(Bytecode& retCtx, const TreeNode& stmt)
{
	unordered_map<string, ScopeManager::SymbolData> captures;
	DetectCaptures(captures, stmt);

	const auto& closureName = stmt.self.val;

	Constant con;
	con._type = Constant::CLOSURE;

	Symbol sym;
	sym.name = closureName;
	sym.pos = 0;
	sym.kind = ESymbol::Var;
	auto dstIdx = _scopeMgr.AddOrNot(sym);
	captures[closureName] = _scopeMgr.GetSymbolData(closureName);

	_scopeMgr.AddClosureScope();
	_scopeMgr.AddLocalScope();
	for(auto& [_, cap] : captures)
	{
		_scopeMgr.AddOrNot(cap.sym);
	}

	bool r = BuildFnReal(con._closure._code, stmt, ESymbol::Var);
	if(!r) return r;

	int srcIdx;
	con._closure._realName = closureName;
	while(1)
	{
		con._closure._uniqueName = closureName; //TODO qaz unique
		if(_constTbl.GetIdx(con) < 0)
		{
			srcIdx = _constTbl.AddOrNot(con);
			break;
		}
	}
	_scopeMgr.PopScope();
	_scopeMgr.PopScope();

	Op::ClosureSet cs{ .dstKind = TO_REF_KIND_U8(dstIdx.kind), .srcKind = (uint8_t)ERefKind::Const, .dst = (uint16_t)dstIdx.idx, .src = (uint16_t)srcIdx };
	retCtx.PushBytecode(cs, stmt.self.line);

	Op::CaptureAdd ca{ .dstKind = TO_REF_KIND_U8(dstIdx.kind), .dst = (uint16_t)dstIdx.idx };
	for(auto& [_, cap] : captures)
	{
		ca.srcKind = TO_REF_KIND_U8(cap.idx.kind);
		ca.src = (uint16_t)cap.idx.idx;
		retCtx.PushBytecode(ca, stmt.self.line);
	}

	return true;
}

bool BytecodeBuilder::BuildFn(Bytecode& retCtx, const TreeNode& stmt)
{
	if(_scopeMgr.GetCurScope() == ScopeManager::SCOPE_LOCAL)
	{
		if(stmt.self.val == "test3")
			int a = 1;
		return BuildClosure(retCtx, stmt);
	}
	else
	{
		return BuildFnReal(retCtx, stmt);
	}
}


void BytecodeBuilder::DetectCaptures(std::unordered_map<std::string, ScopeManager::SymbolData>& retCaptures, const TreeNode& stmt) const
{
	if(stmt.self == EToken::Id && !retCaptures.contains(stmt.self.val))
	{
		auto sym = _scopeMgr.GetSymbolData(stmt.self.val);
		if(!sym.idx.IsNone() && sym.idx.kind == ScopeManager::Idx::LOCAL)
			retCaptures[stmt.self.val] = sym;
		return;
	}

	for(auto& c : stmt.childs)
	{
		DetectCaptures(retCaptures, *c);
	}

	return;
}


}
