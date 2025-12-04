#include "Rand.h"
#include "vm/Variable2.h"
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
	srand( (unsigned int)time( NULL ) );
	RANDOM(0, 1);	//TODO i don't know why
	return {};
}

YRet Seed(YArgs* args)
{
	auto seed = (Variable2*)args->args[0].o;

	srand( seed->int_() );
	return {};
}


YRet Get(YArgs* args)
{
	auto min = (Variable2*)args->args[0].o;
	auto max = (Variable2*)args->args[1].o;

	YRet yr;
	yr.single.tp = YEArg::YVar;
	auto rv = (Variable2*)args->retBuff.o;
	rv->SetInt( RANDOM(min->int_(), max->int_()) );
	yr.single.o = rv;
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
