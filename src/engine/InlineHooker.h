#pragma once

const int A64_MAX_BACKUPS = 256;
const int A64_MAX_INSTRUCTIONS = 5;
const int A64_MAX_REFERENCES = A64_MAX_INSTRUCTIONS * 2;
const int A64_NOP = 0xD503201F;

struct fix_info
{
	uint32_t* bp;
	uint32_t ls; // left-shift counts
	uint32_t ad; // & operand
};
struct insns_info
{
	union
	{
		uint64_t insu;
		int64_t  ins;
		void* insp;
	};
	fix_info fmap[A64_MAX_REFERENCES];
};

typedef uint32_t* __restrict* __restrict instruction;
void A64HookFunction(void* const symbol, void* const replace, void** result);
