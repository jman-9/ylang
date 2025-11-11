#include "Machine.h"
#include <format>
#include <iostream>
using namespace std;

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
		switch(c.type)
		{
		case Constant::INT:		_consts.push_back({ .type = Variable::INT, ._int = c._int }); break;
		case Constant::FLOAT:	_consts.push_back({ .type = Variable::FLOAT, ._float = c._float }); break;
		case Constant::STR:		_consts.push_back({ .type = Variable::STR, .str = c.str }); break;
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
						auto v = ResolveVar(ERefKind::Reg, _rp);
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
						auto v = ResolveVar(ERefKind::Reg, _rp);
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
			if(dst->type != Variable::REF)
			{
				t = dst;
			}
			else
			{
				auto t = dst->ref;
				dst->Clear();
			}
			t->Clear();
			t->list = new std::vector<Variable *>;
			t->type = Variable::LIST;
		}
		else if(inst == EOpcode::ListAdd)
		{
			const Op::ListAdd& la = *(Op::ListAdd*)inst.code.data();
			Variable* src = ResolveVar((ERefKind)la.srcKind, la.src);
			Variable* dst = ResolveVar((ERefKind)la.dstKind, la.dst);
			if(dst->type != Variable::LIST)
			{
				throw 'n';
			}
			dst->list->push_back(src->Clone());
		}
		else if(inst == EOpcode::DictSet)
		{
			const Op::DictSet& ds = *(Op::DictSet*)inst.code.data();
			Variable* dst = ResolveVar((ERefKind)ds.dstKind, ds.dst);

			Variable* t = nullptr;
			if(dst->type != Variable::REF)
			{
				t = dst;
			}
			else
			{
				t = dst->ref;
				dst->Clear();
			}
			t->Clear();
			t->dict = new std::unordered_map<std::string, Variable *>;
			t->type = Variable::DICT;
		}
		else if(inst == EOpcode::DictAdd)
		{
			const Op::DictAdd& da = *(Op::DictAdd*)inst.code.data();
			Variable* val = ResolveVar((ERefKind)da.valKind, da.val);
			Variable* key = ResolveVar((ERefKind)da.keyKind, da.key);
			Variable* dst = ResolveVar((ERefKind)da.dstKind, da.dst);
			if(dst->type != Variable::DICT)
			{
				throw 'n';
			}
			if(key->type != Variable::STR)
			{
				throw 'n';
			}

			(*dst->dict)[key->str] = val->Clone();
		}
		else if(inst == EOpcode::Index)
		{
			const Op::Index& li = *(Op::Index*)inst.code.data();
			Variable* idx = ResolveVar((ERefKind)li.idxKind, li.idx);
			Variable* dst = ResolveVar((ERefKind)li.dstKind, li.dst);

			if(idx->type == Variable::INT)
			{
				if(dst->type != Variable::LIST)
				{
					throw 'n';
				}

				*dst = *dst->list->at(idx->_int);
			}
			else if(idx->type == Variable::STR)
			{
				if(dst->type != Variable::DICT)
				{
					throw 'n';
				}

				auto found = dst->dict->find(idx->str);
				if(found == dst->dict->end())
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

			if(idx->type == Variable::INT)
			{
				if(dst->type != Variable::LIST)
				{
					throw 'n';
				}

				auto t = dst->list->at(idx->_int);
				dst->Clear();
				dst->type = Variable::REF;
				dst->ref = t;
			}
			else if(idx->type == Variable::STR)
			{
				if(dst->type != Variable::DICT)
				{
					throw 'n';
				}

				Variable* t = nullptr;
				auto found = dst->dict->find(idx->str);
				if(found == dst->dict->end())
				{
					auto inserted = dst->dict->insert({idx->str, new Variable()});
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
				dst->type = Variable::REF;
				dst->ref = t;
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

			if(dst->type != Variable::ATTR)
			{
				throw 'n';
			}

			Variable* ret = nullptr;
			if(dst->attr->owner.type == Variable::STR)
			{
				if(dst->attr->name == "len")
				{
					ret = Variable::NewNum(dst->attr->owner.str.size());
				}
				else if(dst->attr->name == "find")
				{
					if(cal.numArgs == 1)
					{
						auto s = ResolveVar(ERefKind::Reg, _rp);
						if(*s != Variable::STR)
						{
							throw 'n';
						}

						size_t pos = dst->attr->owner.str.find(s->str);
						ret = Variable::NewNum(pos == string::npos ? -1 : pos);
					}
					else if(cal.numArgs == 2)
					{
						auto s = ResolveVar(ERefKind::Reg, _rp);
						auto i = ResolveVar(ERefKind::Reg, _rp);
						if(*s != Variable::STR)
						{
							throw 'n';
						}
						if(*i != Variable::INT)
						{
							throw 'n';
						}

						size_t pos = dst->attr->owner.str.find(s->str, s->_int);
						ret = Variable::NewNum(pos == string::npos ? -1 : pos);
					}
					else
					{
						throw 'n';
					}
				}
				else if(dst->attr->name == "substr")
				{
					if(cal.numArgs == 1)
					{
						auto s = ResolveVar(ERefKind::Reg, _rp);
						if(*s != Variable::INT)
						{
							throw 'n';
						}

						ret = Variable::NewStr(dst->attr->owner.str.substr(s->_int));
					}
					else if(cal.numArgs == 2)
					{
						auto s = ResolveVar(ERefKind::Reg, _rp);
						auto l = ResolveVar(ERefKind::Reg, _rp);
						if(*s != Variable::INT)
						{
							throw 'n';
						}
						if(*l != Variable::INT)
						{
							throw 'n';
						}

						ret = Variable::NewStr(dst->attr->owner.str.substr(s->_int, l->_int));
					}
					else
					{
						throw 'n';
					}
				}
				else if(dst->attr->name == "replace")
				{
					auto o = ResolveVar(ERefKind::Reg, _rp);
					auto n = ResolveVar(ERefKind::Reg, _rp);
					if(*o != Variable::STR)
					{
						throw 'n';
					}
					if(*n != Variable::STR)
					{
						throw 'n';
					}

					string r = dst->attr->owner.str;
					if(!o->str.empty())
					{
						size_t pos = 0;
						for(size_t pos=0; (pos = r.find(o->str, pos)) != std::string::npos; ) {
							r.replace(pos, o->str.length(), n->str);
							pos += n->str.length();
						}
						ret = Variable::NewStr(r);
					}
				}
				else if(dst->attr->name == "split")
				{
					if(cal.numArgs == 0)
					{
						const string& src = dst->attr->owner.str;
						size_t start = src.find_first_not_of(" \t\n\r");
						size_t end = 0;
						ret = Variable::NewList();

						for( ; start != string::npos; )
						{
							end = src.find_first_of(" \t\n\r", start);
							ret->list->push_back(Variable::NewStr(src.substr(start, end - start)));
							start = src.find_first_not_of(" \t\n\r", end);
						}
					}
					else if(cal.numArgs == 1)
					{
						auto d = ResolveVar(ERefKind::Reg, _rp);
						if(*d != Variable::STR)
						{
							throw 'n';
						}

						if(d->str.empty())
						{
							ret = Variable::NewStr(dst->attr->owner.str);
						}
						else
						{
							size_t start = 0;
							size_t end = 0;
							ret = Variable::NewList();

							const string& src = dst->attr->owner.str;
							for( ; (end = src.find(d->str, start)) != std::string::npos; )
							{
								ret->list->push_back(Variable::NewStr(src.substr(start, end - start)));
								start = end + d->str.length();
							}
							ret->list->push_back(Variable::NewStr(src.substr(start, end - start)));
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
			if(dst->attr->owner.type == Variable::LIST)
			{
				if(dst->attr->name == "append")
				{
					if(cal.numArgs != 1)
					{
						throw 'n';
					}

					auto v = ResolveVar(ERefKind::Reg, _rp);

					dst->attr->owner.list->push_back(v->Clone());
				}
				else if(dst->attr->name == "insert")
				{
					if(cal.numArgs != 2)
					{
						throw 'n';
					}

					auto i = ResolveVar(ERefKind::Reg, _rp);
					auto v = ResolveVar(ERefKind::Reg, _rp);

					if(*v != Variable::INT)
					{
						throw 'n';
					}

					dst->attr->owner.list->insert(dst->attr->owner.list->begin() + i->_int, v->Clone());
				}
				else if(dst->attr->name == "pop")
				{
					if(cal.numArgs != 1)
					{
						throw 'n';
					}

					auto v = ResolveVar(ERefKind::Reg, _rp);
					if(v->type != Variable::INT)
					{
						throw 'n';
					}

					// todo leak
					ret = dst->attr->owner.list->at(v->_int);
					dst->attr->owner.list->erase(dst->attr->owner.list->begin() + v->_int);
				}
				else if(dst->attr->name == "pop_front")
				{
					// todo leak
					ret = dst->attr->owner.list->front();
					dst->attr->owner.list->erase(dst->attr->owner.list->begin());
				}
				else if(dst->attr->name == "pop_back")
				{
					// todo leak
					ret = dst->attr->owner.list->back();
					dst->attr->owner.list->pop_back();
				}
				else if(dst->attr->name == "len")
				{
					// todo leak
					ret = new Variable;
					ret->type = Variable::INT;
					ret->_int = (int64_t)dst->attr->owner.list->size();
				}
				else
				{
					throw 'n';
				}
			}
			else if(dst->attr->owner.type == Variable::DICT)
			{
				if(dst->attr->name == "keys")
				{
					ret = new Variable;
					ret->type = Variable::LIST;
					ret->list = new vector<Variable*>;

					for(auto& [k, _] : *dst->attr->owner.dict)
					{
						ret->list->push_back(new Variable{ .type = Variable::STR, .str = k });
					}
				}
				else if(dst->attr->name == "values")
				{
					ret = new Variable;
					ret->type = Variable::LIST;
					ret->list = new vector<Variable*>;

					for(auto& [_, v] : *dst->attr->owner.dict)
					{
						ret->list->push_back(v);
					}
				}
				else if(dst->attr->name == "items")
				{
					ret = new Variable;
					ret->type = Variable::LIST;
					ret->list = new vector<Variable*>;

					for(auto& [k, v] : *dst->attr->owner.dict)
					{
						Variable* pair = new Variable;
						pair->type = Variable::LIST;
						pair->list = new vector<Variable*>;
						Variable* kv = new Variable;
						kv->type = Variable::STR;
						kv->str = k;
						pair->list->push_back(kv);
						pair->list->push_back(v);

						ret->list->push_back(pair);
					}
				}
				else if(dst->attr->name == "len")
				{
					// todo leak
					ret = new Variable;
					ret->type = Variable::INT;
					ret->_int = (int64_t)dst->attr->owner.dict->size();
				}
				else if(dst->attr->name == "pop")
				{
					if(cal.numArgs != 1)
					{
						throw 'n';
					}

					auto v = ResolveVar(ERefKind::Reg, _rp);
					if(v->type != Variable::STR)
					{
						throw 'n';
					}

					auto found = dst->attr->owner.dict->find(v->str);
					if(found != dst->attr->owner.dict->end())
					{
						ret = found->second;
						dst->attr->owner.dict->erase(found);
					}
				}
				else
				{
					throw 'n';
				}
			}

			_rp = _rpStack.top();
			_rpStack.pop();

			auto v = ResolveVar(ERefKind::Reg, _rp);
			*v = ret ? *ret : Variable();
		}

		i++;
	}
}

}
