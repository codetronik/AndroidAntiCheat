#pragma once
#include "Module.h"
#include "Util.h"
#include "MyApi.h"
class FileModule : public Module
{
public:
    FileModule(string path);
    virtual ~FileModule();
    bool Init() override;
    void Relocate();
protected:
    bool GetSectionAddr() override;
private:
 
};

