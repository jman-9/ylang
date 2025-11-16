#include "Rand.h"
#include "vm/Variable.h"
#include <time.h>


#define RANDOM(__min__, __max__) ((int)(((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * (((__max__) + 1) - (__min__))) + (__min__))


namespace ybuiltin::Rand
{
using namespace yvm;

YRet RandomizeTimer(YArgs* args)
{
	srand( (unsigned)time( NULL ) );
	return {};
}

YRet Get(YArgs* args)
{
	auto min = (Variable*)args->args[0].o;
	auto max = (Variable*)args->args[1].o;

	YRet yr;
	yr.single.tp = YEArg::YVar;
	yr.single.o = Variable::NewInt( RANDOM(min->_int, max->_int) );
	return yr;
}

const ymod::ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "rand";
		m.builtin = true;
		m.memberTbl[ "randomize_timer" ] = { "randomize_timer", ymod::ModuleMemberDesc::FUNC, false, 0, RandomizeTimer };
		m.memberTbl[ "get" ] = { "get", ymod::ModuleMemberDesc::FUNC, false, 2, Get };
	}
	return m;
}

}
