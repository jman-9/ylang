#pragma once
#include "contract/ycontract.h"
#include "vm/Variable.h"
#include "vm/RuntimeError.h"


#define MODARG_VAR(__idx__, __name__, __type__)			\
	if(__idx__ >= args->numArgs) INTERNALERR(format("{}: out of range in '{}'", __idx__, "args")); \
	const auto& __name__ = *(Variable*)args->args[__idx__].o;	\
	if(__name__ != __type__) throw RuntimeError::IncorrectParam(__name__._type, __type__, __idx__)

#define MODARG_VAR2(__idx__, __name__, __type1__, __type2__)			\
	if(__idx__ >= args->numArgs) INTERNALERR(format("{}: out of range in '{}'", __idx__, "args")); \
	const auto& __name__ = *(Variable*)args->args[__idx__].o;	\
	if(__name__ != __type1__ && __name__ != __type2__) throw RuntimeError::IncorrectParam(__name__._type, {__type1__, __type2__}, __idx__)
