#include "Rand.h"
#include "vm/Variable.h"
#include <time.h>


#ifdef _WIN32
	#define RANDOM(__min__, __max__) ((int)(((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * (((__max__) + 1) - (__min__))) + (__min__))
#else
	#define RANDOM(__min__, __max__) ((int64_t)rand() * ((__max__) + 1 - (__min__)) / ((int64_t)RAND_MAX + 1) + (__min__))
#endif


namespace ybuiltin::Rand
{
using namespace yvm;

YRet RandomizeTimer(YArgs* args)
{
	srand( (unsigned)time( NULL ) );
	return {};
}

YRet Seed(YArgs* args)
{
	auto seed = (Variable*)args->args[0].o;

	srand( seed->_int );
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
		m.memberTbl[ "seed" ] = { "seed", ymod::ModuleMemberDesc::FUNC, false, 1, Seed };
		m.memberTbl[ "get" ] = { "get", ymod::ModuleMemberDesc::FUNC, false, 2, Get };
	}
	return m;
}

}
