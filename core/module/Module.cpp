#include "Module.h"

namespace ymod
{


bool ModuleDesc::IsNull() const
{
	return name.empty();
}

}
