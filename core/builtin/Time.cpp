#include "Time.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"
#include "module/ModuleUtil.h"
#include <time.h>
#include <thread>



namespace ybuiltin::Time
{
using namespace yvm;
using namespace std;


YRet Sleep(YArgs* args)
{
	MODARG_VAR(0, dur, Variable::INT);

	this_thread::sleep_for(std::chrono::milliseconds(dur.int_()));
	return {};
}


YRet Now(YArgs* args)
{
	auto rv = (Variable*)args->retBuff.o;
	rv->SetInt(time(NULL));
	YRet yr;
	yr.single.SetYVar(rv);
	return yr;
}


const ymod::ModuleDesc& GetModuleDesc()
{
	static ymod::ModuleDesc m;

	if(m.name.empty())
	{
		m.name = "time";
		m.builtin = true;
		m.memberTbl[ "sleep" ] = { "sleep", ymod::ModuleMemberDesc::FUNC, false, 1, Sleep };
		m.memberTbl[ "now" ] = { "now", ymod::ModuleMemberDesc::FUNC, false, 0, Now };
	}
	return m;
}

}
