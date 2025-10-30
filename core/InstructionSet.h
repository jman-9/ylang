#pragma once
#include <stdint.h>

enum class EOpcode : uint16_t
{
	Noop,

	Assign,
	PushSp,
	PopSp,
	Jmp,
	Invoke,
	Ret,
	Jz,
};

enum class ERefKind : uint8_t
{
	None,

	Const,
	Reg,
	GlobalVar,
	LocalVar,
};

namespace Inst
{

struct Assign
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t src1Kind = (uint8_t)ERefKind::None;
	uint8_t src2Kind = (uint8_t)ERefKind::None;
	uint8_t op = (uint8_t)EToken::None;
	uint16_t dst = 0;
	uint16_t src1 = 0;
	uint16_t src2 = 0;
	uint16_t rsvd = 0;
};

struct Jmp
{
	uint32_t pos = 0;
};

struct Invoke
{
	uint32_t pos = 0;
	uint32_t numPrms = 0;
};

struct Jz
{
	uint8_t testKind = (uint8_t)ERefKind::None;
	uint8_t rsvd = 0;
	uint16_t test = 0;
	uint32_t pos = 0;
};

}
