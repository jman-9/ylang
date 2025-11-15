#pragma once
#include "Token.h"
#include <stdint.h>

enum class EOpcode : uint16_t
{
	Noop,

	Assign,
	PushSp,
	PopSp,
	Jmp,
	Call,
	Ret,
	Jz,
	ListSet,
	ListAdd,
	DictSet,
	DictAdd,
	Index,
	LValueIndex,
	Invoke,
	Inc,
};

enum class ERefKind : uint8_t
{
	None,

	Const,
	Reg,
	GlobalVar,
	LocalVar,
};

namespace Op
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

struct Call
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

struct ListSet
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t rsvd = 0;
	uint16_t dst = 0;
};

struct ListAdd
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t srcKind = (uint8_t)ERefKind::None;
	uint16_t dst = 0;
	uint16_t src = 0;
};

struct DictSet
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t rsvd = 0;
	uint16_t dst = 0;
};

struct DictAdd
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t keyKind = (uint8_t)ERefKind::None;
	uint8_t valKind = (uint8_t)ERefKind::None;
	uint8_t rsvd = 0;
	uint16_t dst = 0;
	uint16_t key = 0;
	uint16_t val = 0;
	uint16_t rsvd2 = 0;
};

struct Index
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t idxKind = (uint8_t)ERefKind::None;
	uint16_t dst = 0;
	uint16_t idx = 0;
};

struct LValueIndex
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint8_t idxKind = (uint8_t)ERefKind::None;
	uint16_t dst = 0;
	uint16_t idx = 0;
};

struct Invoke
{
	uint8_t dstKind = (uint8_t)ERefKind::None;
	uint16_t dst = 0;
	uint8_t numArgs = 0;
};

struct Inc
{
	uint16_t inc = 0;
	uint16_t rsvd = 0;
};


}
