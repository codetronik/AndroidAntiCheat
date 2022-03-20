#include "InlineStatus.h"
#include "Log.h"
InlineStatus::InlineStatus(intptr_t addr)
{		
	this->addr = addr;
}
InlineStatus::InlineStatus(const InlineStatus& r)
{
	this->addr = r.addr;
}
InlineStatus::operator intptr_t()
{	
	return addr;
}
bool InlineStatus::IsHooked()
{	
	// test is here : https://armconverter.com/?disasm

	if (*((unsigned char*)(addr + 3)) >= 0x14 
		&& *((unsigned char*)(addr + 3)) <= 0x17) // B 명령어 사용
	{
		return true;
	}
	else if (*(uint32_t*)addr == 0xd503201f) // NOP
	{
		return true;
	}
	return false;
}


