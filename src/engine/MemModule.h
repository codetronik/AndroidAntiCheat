#pragma once
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

