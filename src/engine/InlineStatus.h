#pragma once
#include <iostream>

class InlineStatus
{
public:
	InlineStatus() = delete;
	InlineStatus(intptr_t addr);
	InlineStatus(const InlineStatus& r);
	operator intptr_t();
	bool IsHooked();
private:
	intptr_t addr;
};
