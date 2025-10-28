#pragma once
#include <stdint.h>

enum class Opcode : uint16_t
{
	Assign,
	PushSp,
	PopSp,
	Jmp,
	Invoke,
	Ret,
};

enum class RefKind : uint8_t
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
	uint8_t dstKind = (uint8_t)RefKind::None;
	uint8_t src1Kind = (uint8_t)RefKind::None;
	uint8_t src2Kind = (uint8_t)RefKind::None;
	uint8_t op = 0;
	uint16_t dst = 0;
	uint16_t src1 = 0;
	uint16_t src2 = 0;
	uint16_t rsvd = 0;
};

struct Jmp
{
	uint32_t pos;
};

struct Invoke
{
	uint32_t pos;
	uint32_t numPrms;
};

}
