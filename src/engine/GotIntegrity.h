#pragma once
#include "MemModule.h"
#include <vector>


struct CheckFunc
{
    const char* funcName;
    ModuleAddr modAddr;
};


class GotIntegrity
{
public:
    GotIntegrity(shared_ptr<MemModule> targetMod);
    bool AddCheckFuncion(const char* func, shared_ptr<MemModule> checkMod);

public:
    bool IsHooked();

private:
    shared_ptr<MemModule> targetMod;
    vector<CheckFunc> checkList;
  
};