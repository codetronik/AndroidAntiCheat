#pragma once
#include <sstream>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include "Module.h"

class MemModule : public Module
{
public:
	MemModule(string path);
	virtual ~MemModule();
	bool Init() override;
private:
	void GetBaseAddress();
protected:
	bool GetSectionAddr() override;
};

