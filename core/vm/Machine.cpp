#include "Machine.h"
#include "module/Module.h"
#include <format>
#include <iostream>
using namespace std;


YRet Sin(YArgs* args)
{//TODO int value check
	double x = args->args[0].ToDouble();
	double v = sin(x);
	YRet yr;
	yr.single.FromDouble(v);
	return yr;
}


namespace yvm
{

Machine::Machine()
{
	_rp = 0;
	_sp = 0;
	_spStack.push(0);
	_rpStack.push(0);
	_cspStack.push(0);
	_regs.resize(1000);
	_stack.resize(5000);
}


Variable* Machine::ResolveVar(ERefKind k, int idx)
{
	switch(k)
	{
	case ERefKind::Reg:
		{
			_rp = _rpStack.top() + idx + 1;
			return &_regs[_rpStack.top() + idx];
		}
	case ERefKind::GlobalVar: return &_stack[idx];
	case ERefKind::LocalVar:
		{
			if(_sp < _cspStack.top() + idx + 1)
				_sp = _cspStack.top() + idx + 1;
			return &_stack[idx + _cspStack.top()];
		}
	case ERefKind::Const: return &_consts[idx];
	default: return nullptr;
	}
}


void Machine::Run(const Bytecode& code, int start /* = 0 */)
{
	_consts.clear();
	for(auto& c : code._consts)
	{
		switch(c._type)
		{
		case Constant::INT:		_consts.push_back({ ._type = Variable::INT, ._int = c._int }); break;
		case Constant::FLOAT:	_consts.push_back({ ._type = Variable::FLOAT, ._float = c._float }); break;
		case Constant::STR:		_consts.push_back({ ._type = Variable::STR, ._str = c._str }); break;
		default: //TODO
			throw 'n';
		}
	}

	int i;
	for(i=start; i<code._code.size(); )
	{
		auto& inst = code._code[i];
		if(inst == EOpcode::Assign)
		{
			const Op::Assign& as = *(Op::Assign*)inst.code.data();

			if((ERefKind)as.dstKind == ERefKind::Const)
			{
				throw 'n';
			}

			if((ERefKind)as.dstKind != ERefKind::None)
			{
				Variable* src1 = ResolveVar((ERefKind)as.src1Kind, as.src1);
				Variable* src2 = ResolveVar((ERefKind)as.src2Kind, as.src2);
				Variable* dst = ResolveVar((ERefKind)as.dstKind, as.dst);
				if(src1 && src2)
				{
					dst->CalcAndAssign(*src1, (EToken)as.op, *src2);
				}
				else if(src1)
				{
					dst->Assign(EToken::Assign, *src1);
				}
				else if(src2)
				{
					dst->CalcUnaryAndAssign((EToken)as.op, *src2);
				}
				else
				{
					throw 'n';
				}
			}
			else
			{
				if(Token::IsAssign((EToken)as.op))
				{
					if((ERefKind)as.src1Kind == ERefKind::Const)
					{
						throw 'n';
					}

					Variable* src = ResolveVar((ERefKind)as.src2Kind, as.src2);
					Variable* dst = ResolveVar((ERefKind)as.src1Kind, as.src1);
					dst->Assign((EToken)as.op, *src);
				}
				else
				{//TODO func call, unary
					throw 'n';
				}
			}
		}
		else if(inst == EOpcode::PushSp)
		{
			_spStack.push(_sp);
		}
		else if(inst == EOpcode::PopSp)
		{
			_sp = _spStack.top();
			_spStack.pop();
		}
		else if(inst == EOpcode::Jmp)
		{
			const Op::Jmp& jmp = *(Op::Jmp*)inst.code.data();
			i = jmp.pos;
			continue;
		}
		else if(inst == EOpcode::Invoke)
		{
			const Op::Invoke& ivk = *(Op::Invoke*)inst.code.data();

			_rp -= (int)ivk.numPrms;
			_rpStack.push(_rp);

			if(ivk.pos >= 0xFFFF0000)
			{// todo new architecture
				if(ivk.pos == 0xFFFF0000)
				{
					if(ivk.numPrms == 0)
					{//noop
					}
					else
					{
						auto v = ResolveVar(ERefKind::Reg, 0);
						cout << v->ToStr();
					}
				}
				else if(ivk.pos == 0xFFFF0000 + 1)
				{
					if(ivk.numPrms == 0)
					{
						cout << "\n";
					}
					else
					{
						auto v = ResolveVar(ERefKind::Reg, 0);
						cout << v->ToStr() << "\n";
					}
				}
				else
				{
					throw 'n';
				}
				_rp = _rpStack.top();
				_rpStack.pop();
			}
			else
			{
				_retStack.push((uint32_t)i+1);
				_cspStack.push(_sp);

				i = ivk.pos;
				continue;
			}
		}
		else if(inst == EOpcode::Ret)
		{
			_sp = _cspStack.top();
			_cspStack.pop();
			_rp = _rpStack.top() + 1;
			_rpStack.pop();
			i = _retStack.top();
			_retStack.pop();
			continue;
		}
		else if(inst == EOpcode::Jz)
		{
			const Op::Jz& jz = *(Op::Jz*)inst.code.data();
			Variable* test = ResolveVar((ERefKind)jz.testKind, jz.test);
			if(!test->_int)
			{
				i = jz.pos;
				continue;
			}
		}
		else if(inst == EOpcode::ListSet)
		{
			const Op::ListSet& ls = *(Op::ListSet*)inst.code.data();
			Variable* dst = ResolveVar((ERefKind)ls.dstKind, ls.dst);

			Variable* t = nullptr;
			if(dst->_type != Variable::REF)
			{
				t = dst;
			}
			else
			{
				auto t = dst->_ref;
				dst->Clear();
			}
			t->Clear();
			t->_list = new std::vector<Variable *>;
			t->_type = Variable::LIST;
		}
		else if(inst == EOpcode::ListAdd)
		{
			const Op::ListAdd& la = *(Op::ListAdd*)inst.code.data();
			Variable* src = ResolveVar((ERefKind)la.srcKind, la.src);
			Variable* dst = ResolveVar((ERefKind)la.dstKind, la.dst);
			if(dst->_type != Variable::LIST)
			{
				throw 'n';
			}
			dst->_list->push_back(src->Clone());
		}
		else if(inst == EOpcode::DictSet)
		{
			const Op::DictSet& ds = *(Op::DictSet*)inst.code.data();
			Variable* dst = ResolveVar((ERefKind)ds.dstKind, ds.dst);

			Variable* t = nullptr;
			if(dst->_type != Variable::REF)
			{
				t = dst;
			}
			else
			{
				t = dst->_ref;
				dst->Clear();
			}
			t->Clear();
			t->_dict = new std::unordered_map<std::string, Variable *>;
			t->_type = Variable::DICT;
		}
		else if(inst == EOpcode::DictAdd)
		{
			const Op::DictAdd& da = *(Op::DictAdd*)inst.code.data();
			Variable* val = ResolveVar((ERefKind)da.valKind, da.val);
			Variable* key = ResolveVar((ERefKind)da.keyKind, da.key);
			Variable* dst = ResolveVar((ERefKind)da.dstKind, da.dst);
			if(dst->_type != Variable::DICT)
			{
				throw 'n';
			}
			if(key->_type != Variable::STR)
			{
				throw 'n';
			}

			(*dst->_dict)[key->_str] = val->Clone();
		}
		else if(inst == EOpcode::Index)
		{
			const Op::Index& li = *(Op::Index*)inst.code.data();
			Variable* idx = ResolveVar((ERefKind)li.idxKind, li.idx);
			Variable* dst = ResolveVar((ERefKind)li.dstKind, li.dst);

			if(idx->_type == Variable::INT)
			{
				if(dst->_type != Variable::LIST)
				{
					throw 'n';
				}

				*dst = *dst->_list->at(idx->_int);
			}
			else if(idx->_type == Variable::STR)
			{
				if(dst->_type != Variable::DICT)
				{
					throw 'n';
				}

				auto found = dst->_dict->find(idx->_str);
				if(found == dst->_dict->end())
				{
					throw 'n';
				}

				*dst = *found->second;
			}
			else
			{
				throw 'n';
			}
		}
		else if(inst == EOpcode::LValueIndex)
		{
			const Op::LValueIndex& lli = *(Op::LValueIndex*)inst.code.data();
			Variable* idx = ResolveVar((ERefKind)lli.idxKind, lli.idx);
			Variable* dst = ResolveVar((ERefKind)lli.dstKind, lli.dst);

			if(idx->_type == Variable::INT)
			{
				if(dst->_type != Variable::LIST)
				{
					throw 'n';
				}

				auto t = dst->_list->at(idx->_int);
				dst->Clear();
				dst->_type = Variable::REF;
				dst->_ref = t;
			}
			else if(idx->_type == Variable::STR)
			{
				if(dst->_type != Variable::DICT)
				{
					throw 'n';
				}

				Variable* t = nullptr;
				auto found = dst->_dict->find(idx->_str);
				if(found == dst->_dict->end())
				{
					auto inserted = dst->_dict->insert({idx->_str, new Variable()});
					if(!inserted.second)
					{
						throw 'n';
					}
					t = inserted.first->second;
				}
				else
				{
					t = found->second;
				}

				dst->Clear();
				dst->_type = Variable::REF;
				dst->_ref = t;
			}
			else
			{
				throw 'n';
			}
		}
		else if(inst == EOpcode::Call)
		{
			const Op::Call& cal = *(Op::Call*)inst.code.data();

			_rp = _rp - cal.numArgs - 1;
			_rpStack.push(_rp);

			Variable* dst = ResolveVar((ERefKind)cal.dstKind, cal.dst);

			if(dst->_type != Variable::ATTR)
			{
				throw 'n';
			}

			Variable* ret = nullptr;
			if(dst->_attr->owner._type == Variable::MODULE)
			{
				auto found = dst->_attr->owner._mod.funcTbl.find(dst->_attr->name);
				if(found == dst->_attr->owner._mod.funcTbl.end())
				{//TODO
					throw 'n';
				}

				if(cal.numArgs < found->second.numPrms)
				{//TODO
					throw 'n';
				}

				YArgs ya;
				ya.Reset(found->second.numPrms);
				for(int i=0; i<found->second.numPrms; i++)
				{//TODO int value check
					auto arg = ResolveVar(ERefKind::Reg, i+1);
					ya.args[i] = arg->ToContract();
				}
				auto yr = found->second.func(&ya);
				if(yr.code)
				{//TODO
					throw 'n';
				}
				if(yr.single.tp != YEObj::None)//TODO real val
				{
					ret = Variable::New(yr.single);
				}
				else if(yr.vals.sz != 0)
				{//TODO
				}
				else
				{//TODO
					throw 'n';
				}
			}
			else if(dst->_attr->owner._type == Variable::STR)
			{
				if(dst->_attr->name == "len")
				{
					ret = Variable::NewNum(dst->_attr->owner._str.size());
				}
				else if(dst->_attr->name == "find")
				{
					if(cal.numArgs == 1)
					{
						auto s = ResolveVar(ERefKind::Reg, 1);
						if(*s != Variable::STR)
						{
							throw 'n';
						}

						size_t pos = dst->_attr->owner._str.find(s->_str);
						ret = Variable::NewNum(pos == string::npos ? -1 : pos);
					}
					else if(cal.numArgs == 2)
					{
						auto i = ResolveVar(ERefKind::Reg, 1);
						auto s = ResolveVar(ERefKind::Reg, 2);
						if(*s != Variable::STR)
						{
							throw 'n';
						}
						if(*i != Variable::INT)
						{
							throw 'n';
						}

						size_t pos = dst->_attr->owner._str.find(s->_str, s->_int);
						ret = Variable::NewNum(pos == string::npos ? -1 : pos);
					}
					else
					{
						throw 'n';
					}
				}
				else if(dst->_attr->name == "substr")
				{
					if(cal.numArgs == 1)
					{
						auto s = ResolveVar(ERefKind::Reg, 1);
						if(*s != Variable::INT)
						{
							throw 'n';
						}

						ret = Variable::NewStr(dst->_attr->owner._str.substr(s->_int));
					}
					else if(cal.numArgs == 2)
					{
						auto s = ResolveVar(ERefKind::Reg, 1);
						auto l = ResolveVar(ERefKind::Reg, 2);
						if(*s != Variable::INT)
						{
							throw 'n';
						}
						if(*l != Variable::INT)
						{
							throw 'n';
						}

						ret = Variable::NewStr(dst->_attr->owner._str.substr(s->_int, l->_int));
					}
					else
					{
						throw 'n';
					}
				}
				else if(dst->_attr->name == "replace")
				{
					auto o = ResolveVar(ERefKind::Reg, 1);
					auto n = ResolveVar(ERefKind::Reg, 2);
					if(*o != Variable::STR)
					{
						throw 'n';
					}
					if(*n != Variable::STR)
					{
						throw 'n';
					}

					string r = dst->_attr->owner._str;
					if(!o->_str.empty())
					{
						size_t pos = 0;
						for(size_t pos=0; (pos = r.find(o->_str, pos)) != std::string::npos; ) {
							r.replace(pos, o->_str.length(), n->_str);
							pos += n->_str.length();
						}
						ret = Variable::NewStr(r);
					}
				}
				else if(dst->_attr->name == "split")
				{
					if(cal.numArgs == 0)
					{
						const string& src = dst->_attr->owner._str;
						size_t start = src.find_first_not_of(" \t\n\r");
						size_t end = 0;
						ret = Variable::NewList();

						for( ; start != string::npos; )
						{
							end = src.find_first_of(" \t\n\r", start);
							ret->_list->push_back(Variable::NewStr(src.substr(start, end - start)));
							start = src.find_first_not_of(" \t\n\r", end);
						}
					}
					else if(cal.numArgs == 1)
					{
						auto d = ResolveVar(ERefKind::Reg, 1);
						if(*d != Variable::STR)
						{
							throw 'n';
						}

						if(d->_str.empty())
						{
							ret = Variable::NewStr(dst->_attr->owner._str);
						}
						else
						{
							size_t start = 0;
							size_t end = 0;
							ret = Variable::NewList();

							const string& src = dst->_attr->owner._str;
							for( ; (end = src.find(d->_str, start)) != std::string::npos; )
							{
								ret->_list->push_back(Variable::NewStr(src.substr(start, end - start)));
								start = end + d->_str.length();
							}
							ret->_list->push_back(Variable::NewStr(src.substr(start, end - start)));
						}
					}
					else
					{
						throw 'n';
					}
				}
				else
				{
					throw 'n';
				}
			}
			if(dst->_attr->owner._type == Variable::LIST)
			{
				if(dst->_attr->name == "append")
				{
					if(cal.numArgs != 1)
					{
						throw 'n';
					}

					auto v = ResolveVar(ERefKind::Reg, 1);

					dst->_attr->owner._list->push_back(v->Clone());
				}
				else if(dst->_attr->name == "insert")
				{
					if(cal.numArgs != 2)
					{
						throw 'n';
					}

					auto i = ResolveVar(ERefKind::Reg, 1);
					auto v = ResolveVar(ERefKind::Reg, 2);

					if(*v != Variable::INT)
					{
						throw 'n';
					}

					dst->_attr->owner._list->insert(dst->_attr->owner._list->begin() + i->_int, v->Clone());
				}
				else if(dst->_attr->name == "pop")
				{
					if(cal.numArgs != 1)
					{
						throw 'n';
					}

					auto v = ResolveVar(ERefKind::Reg, 1);
					if(v->_type != Variable::INT)
					{
						throw 'n';
					}

					// todo leak
					ret = dst->_attr->owner._list->at(v->_int);
					dst->_attr->owner._list->erase(dst->_attr->owner._list->begin() + v->_int);
				}
				else if(dst->_attr->name == "pop_front")
				{
					// todo leak
					ret = dst->_attr->owner._list->front();
					dst->_attr->owner._list->erase(dst->_attr->owner._list->begin());
				}
				else if(dst->_attr->name == "pop_back")
				{
					// todo leak
					ret = dst->_attr->owner._list->back();
					dst->_attr->owner._list->pop_back();
				}
				else if(dst->_attr->name == "len")
				{
					// todo leak
					ret = new Variable;
					ret->_type = Variable::INT;
					ret->_int = (int64_t)dst->_attr->owner._list->size();
				}
				else
				{
					throw 'n';
				}
			}
			else if(dst->_attr->owner._type == Variable::DICT)
			{
				if(dst->_attr->name == "keys")
				{
					ret = new Variable;
					ret->_type = Variable::LIST;
					ret->_list = new vector<Variable*>;

					for(auto& [k, _] : *dst->_attr->owner._dict)
					{
						ret->_list->push_back(new Variable{ ._type = Variable::STR, ._str = k });
					}
				}
				else if(dst->_attr->name == "values")
				{
					ret = new Variable;
					ret->_type = Variable::LIST;
					ret->_list = new vector<Variable*>;

					for(auto& [_, v] : *dst->_attr->owner._dict)
					{
						ret->_list->push_back(v);
					}
				}
				else if(dst->_attr->name == "items")
				{
					ret = new Variable;
					ret->_type = Variable::LIST;
					ret->_list = new vector<Variable*>;

					for(auto& [k, v] : *dst->_attr->owner._dict)
					{
						Variable* pair = new Variable;
						pair->_type = Variable::LIST;
						pair->_list = new vector<Variable*>;
						Variable* kv = new Variable;
						kv->_type = Variable::STR;
						kv->_str = k;
						pair->_list->push_back(kv);
						pair->_list->push_back(v);

						ret->_list->push_back(pair);
					}
				}
				else if(dst->_attr->name == "len")
				{
					// todo leak
					ret = new Variable;
					ret->_type = Variable::INT;
					ret->_int = (int64_t)dst->_attr->owner._dict->size();
				}
				else if(dst->_attr->name == "pop")
				{
					if(cal.numArgs != 1)
					{
						throw 'n';
					}

					auto v = ResolveVar(ERefKind::Reg, 1);
					if(v->_type != Variable::STR)
					{
						throw 'n';
					}

					auto found = dst->_attr->owner._dict->find(v->_str);
					if(found != dst->_attr->owner._dict->end())
					{
						ret = found->second;
						dst->_attr->owner._dict->erase(found);
					}
				}
				else
				{
					throw 'n';
				}
			}

			_rpStack.pop();
			auto v = ResolveVar(ERefKind::Reg, 0);
			*v = ret ? *ret : Variable();
		}
		else if(inst == EOpcode::Inc)
		{
			const Op::Inc& inc = *(Op::Inc*)inst.code.data();
			Variable* name = ResolveVar(ERefKind::Const, inc.inc);

			if(!name || *name != Variable::STR)
			{
				throw 'n';
			}

			ymod::Module m;
			m.name = "math";
			m.funcTbl[ "sin" ] =  { "sin", 1, Sin};

			auto v = ResolveVar(ERefKind::LocalVar, _sp);
			v->SetModule(m);
		}


		i++;
	}
}

}
