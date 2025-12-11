#include "Rand.h"
#include "vm/Variable.h"
#include <time.h>


#ifdef _WIN32
	#define RANDOM(__min__, __max__) ((int)(((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * (((__max__) + 1) - (__min__))) + (__min__))

	#define RANDOMD(__min__, __max__) ((((double)((rand()<<15) | (rand()&0) | rand())) / ((RAND_MAX<<15 | RAND_MAX) + 1) * ((((double)__max__)) - (double)(__min__))) + (double)(__min__))
#else
	#define RANDOM(__min__, __max__) ((int64_t)rand() * ((__max__) + 1 - (__min__)) / ((int64_t)RAND_MAX + 1) + (__min__))
	#define RANDOMD(__min__, __max__) ((double)rand() / ((int64_t)RAND_MAX + 1) *  (((double)(__max__)) - ((double)(__min__)))+ (double)(__min__))
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
	auto seed = (Variable*)args->args[0].o;

	srand( seed->int_() );
	return {};
}


YRet Get(YArgs* args)
{
	auto vmin = *(Variable*)args->args[0].o;
	auto vmax = *(Variable*)args->args[1].o;

	auto rv = (Variable*)args->retBuff.o;
	if(vmin == Variable::INT && vmax == Variable::INT)
	{
		rv->SetInt( RANDOM(vmin.int_(), vmax.int_()) );
	}
	else
	{
		double min = vmin == Variable::INT ? (double)vmin.int_() : vmin.float_();
		double max = vmax == Variable::INT ? (double)vmax.int_() : vmax.float_();
		rv->SetFloat( RANDOMD(min, max) );
	}

	YRet yr;
	yr.single.SetYVar(rv);
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
