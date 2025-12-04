#pragma once
#include "../Program.h"
#include "module/Module.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>


namespace yvm
{
struct Attribute2;
struct Variable2;

struct ClassObject2
{
	std::vector<Variable2> _fields;
	const Class* _cls;
};

struct Variable2
{
	enum Type
	{
		NONE,

		INT,
		FLOAT,
		STR,
		REF,
		LVREF,
		ATTR,
		CLASS,
		MODULE,

		LIST,
		DICT,
		CLASSOBJ,
		MODULEOBJ,

		_NULL_,
		_TRUE_,
		_FALSE_,
	};

	Type _type;

	struct Object;
	union u
	{
		int64_t _i;
		double _f;
		std::string* _s;
		Object* _o;
		Variable2* _ref;
		Attribute2* _attr;
		//Class* _cls;	 // TODO to run static method
		ymod::ModuleDesc* _mod;
	} _u;


	Variable2();
    Variable2(const Variable2& v);
	~Variable2();

	void Clear();

	void SetInt(int64_t i);
	void SetFloat(double f);
	void SetStr(std::string s);
	void SetVarRef(Variable2& ref);
	void SetVarLVRef(Variable2& lvref);
	void SetAttr(Variable2& owner, std::string name);
	void SetAttr(Attribute2& attr);
	void SetList(const std::vector<Variable2>& list = std::vector<Variable2>());
	void SetDict(const std::unordered_map<std::string, Variable2>& dict = std::unordered_map<std::string, Variable2>());
	void SetClass(const Class& cls, bool makeInstance);
	void SetModule(const ymod::ModuleDesc& mod, bool makeInstance);
	void SetVar(Variable2& var);

	void SetValueFromContract(YArg o);

	bool Assign(EToken op, Variable2& rval);
	bool CalcAndAssign(Variable2& lhs, EToken calcOp, Variable2& rhs);
	bool CalcUnaryAndAssign(EToken unaryOp, Variable2& rhs);
	bool CalcIncDec(EToken op);

	std::string ToStr() const;

	bool operator==(Type cmp) const;
	bool operator!=(Type cmp) const;
	const Variable2& operator=(const Variable2& rhs);

	void ResetNewRef();

	int64_t int_() const;
	double float_() const;
	const std::string& str() const;
	Variable2& ref();
	const Variable2& ref() const;
	const Attribute2& attr() const;
	Attribute2& attr();
	const ymod::ModuleDesc& mod() const;
	std::vector<Variable2>& list();
	std::unordered_map<std::string, Variable2>& dict();
	const ClassObject2& clsObj() const;
	ClassObject2& clsObj();
	const ymod::Module& modObj() const;
	ymod::Module& modObj();


	struct Object
	{
		std::vector<Variable2> _list;
		std::unordered_map<std::string, Variable2> _dict;
		ClassObject2 _clso;
		ymod::Module _modo;

	private:
		friend struct Variable2;
		Object();
		~Object();

		void AddRef();
		void ReleaseRef();

		int _refCnt;
		Variable2::Type _type;
	};

private:
	void SetObj(Object* obj);
};


struct Attribute2
{
	Variable2 owner;
	std::string name;
};

}
