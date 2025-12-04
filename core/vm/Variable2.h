#pragma once
#include "../Program.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>


namespace yvm
{

struct Variable2;
struct ClassObject2
{
	std::vector<Variable2> _fields;
	Class* _cls;
};
struct Attribute
{
	Variable2& owner;
	std::string name;
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
		MODULE,
		LIST,
		DICT,
		CLASS,
		CLASSOBJ,
	};

	Type _type;

	struct Object;
	union u
	{
		int64_t _i;
		double _f;
		std::string* _s;
		Object* _obj;
		Variable2* _ref;
		Attribute* _attr;
		//Class* _cls;	 // TODO to run static method
	} _u;


	Variable2();

	void Clear();

	void SetInt(int64_t i);
	void SetFloat(double f);
	void SetStr(const std::string& s);
	void SetVarRef(Variable2& ref);
	void SetVarLVRef(Variable2& lvref);
	void SetAttr(Attribute& attr);
	void SetVar(Variable2& var);

	void ResetNewRef();


	struct Object
	{
		std::vector<Variable2> _list;
		std::unordered_map<std::string, Variable2> _dict;
		ClassObject2 _clsObj;

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

}
