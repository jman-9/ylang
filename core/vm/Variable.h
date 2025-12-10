#pragma once
#include "contract/Program.h"
#include "module/Module.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>


namespace yvm
{
struct Attribute;
struct Variable;

struct ClassObject
{
	std::vector<Variable> _fields;
	std::vector<Variable> _prgObj;
	const Class* _cls;
};
struct ModuleObject
{
	ymod::Module _mod;
	void* _o;
};
struct ProgramObject
{
	int _lsp;
	std::vector<Variable> _local;
	std::vector<Variable> _consts;
	const Program* _prg;
};


struct Variable
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
		PROGRAM,

		OBJ,
		LIST,
		DICT,
		CLASSOBJ,
		MODULEOBJ,
		PROGRAMOBJ,

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
		Variable* _ref;
		Attribute* _attr;
		const Class* _cls;
		const Program* _prg;
		//ymod::ModuleDesc* _mod; //TODO to separate module and moduleobj
	} _u;


	Variable();
	~Variable();

	Variable(const Variable& v);
	const Variable& operator=(const Variable& rhs);
	void Clear();

	void SetInt(int64_t i);
	void SetFloat(double f);
	void SetStr(std::string s);
	void SetVarRef(Variable& ref);
	void SetVarLVRef(Variable& lvref);
	void SetAttr(Variable& owner, std::string name);
	void SetAttr(Attribute& attr);
	void SetList(const std::vector<Variable>& list = std::vector<Variable>());
	void SetDict(const std::unordered_map<std::string, Variable>& dict = std::unordered_map<std::string, Variable>());
	void SetClass(const Class& cls, Variable* prgObj, bool makeInstance);
	void SetModule(const ymod::ModuleDesc& mod, bool makeInstance);
	void SetProgram(const Program& prg, bool makeInstance);
	void SetVar(Variable& var);

	bool Assign(EToken op, Variable& rval);
	bool CalcAndAssign(Variable& lhs, EToken calcOp, Variable& rhs);
	bool CalcUnaryAndAssign(EToken unaryOp, Variable& rhs);
	bool CalcIncDec(EToken op);

	std::string ToStr() const;

	bool IsObject() const;
	bool IsNullOrFalse() const;
	bool operator==(Type cmp) const;
	bool operator!=(Type cmp) const;

	void ResetNewObj();

	int64_t int_() const;
	double float_() const;
	const std::string& str() const;
	Variable& ref();
	const Variable& ref() const;
	const Attribute& attr() const;
	Attribute& attr();
	const Class& cls() const;
	const ymod::ModuleDesc& mod() const;
	const Program& prg() const;
	const std::vector<Variable>& list() const;
	std::vector<Variable>& list();
	const std::unordered_map<std::string, Variable>& dict() const;
	std::unordered_map<std::string, Variable>& dict();
	const ClassObject& clsObj() const;
	ClassObject& clsObj();
	const ModuleObject& modObj() const;
	ModuleObject& modObj();
	const ProgramObject& prgObj() const;
	ProgramObject& prgObj();

	void SetValueFromContract(YArg o);
	YArg ToContract() const;

	class Object
	{
		friend struct Variable;

		std::vector<Variable> _list;
		std::unordered_map<std::string, Variable> _dict;
		ClassObject _clso;
		ModuleObject _modo;
		ProgramObject _prgo;

		Object();
		~Object();

		void AddRef();
		void ReleaseRef();

		int _refCnt;
		Variable::Type _type;
	};

private:
	void SetObj(Object* obj);
};


struct Attribute
{
	Variable owner;
	std::string name;
};

}
