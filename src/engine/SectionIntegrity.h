#pragma once
#include "MemModule.h"
#include "FileModule.h"
#include "Log.h"

class SectionIntegrity
{
public:
    SectionIntegrity() = delete;
    SectionIntegrity(shared_ptr<MemModule> targetMod);
    ~SectionIntegrity();
    bool Init();
    bool IsModified(string section);

private:
    bool CheckSectionIntegrity(string section);

private:
    shared_ptr<MemModule> targetMod;    
    optional<ModuleAddr> fileModAddr;
};
