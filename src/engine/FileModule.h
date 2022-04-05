#pragma once
#include <sstream>
#include <dlfcn.h>
#include <link.h>
#include "Module.h"
#include "MemModule.h"
#include "Util.h"
#include "MyApi.h"

class FileModule : public Module
{
public:
	FileModule(string path);
	virtual ~FileModule();
	bool Init() override;
	void SetExecutable();
protected:
	bool GetSectionAddr() override;
private:
	bool executable;
};

