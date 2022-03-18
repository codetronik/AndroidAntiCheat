#pragma once
#include <iostream>
#include <elf.h>
#include <vector>
#include <optional>
#include "Log.h"
#include "Mmap.h"
#include "Util.h"

using namespace std;

struct ModuleAddr
{
    intptr_t startAddr;
    intptr_t codeSectionStartAddr;
    intptr_t codeSectionEndAddr;
    intptr_t gotSectionStartAddr;
    intptr_t gotSectionEndAddr;
    intptr_t gotPltSectionStartAddr;
    intptr_t gotPltSectionEndAddr;
    intptr_t dynsymSectionStartAddr;
    intptr_t dynsymSectionEndAddr;
    intptr_t dynstrSectionStartAddr;
    intptr_t dynstrSectionEndAddr;
    intptr_t reladynSectionStartAddr;
    intptr_t reladynSectionEndAddr;
    intptr_t relapltSectionStartAddr;
    intptr_t relapltSectionEndAddr; 
    intptr_t rodataSectionStartAddr;
    intptr_t rodataSectionEndAddr;
};

struct Entry
{
    char* funcName;
    intptr_t funcAddr;
};
struct GotEntry
{
    char* funcName;
    intptr_t gotAddr;
    intptr_t funcAddr;
    bool isHooked;
};

class Module
{    
public:
    Module(string path);
    virtual ~Module();
    virtual bool Init() = 0;
    optional<ModuleAddr> GetModuleAddr();
    vector<GotEntry> GetGotPltEntries();
    vector<GotEntry> GetGotEntries();

protected:
    void GetRelaDynEntries();
    void GetDynSymEntries();
    void GetRelaPltEntries();
    virtual bool GetSectionAddr() = 0;

public:
    string path;

protected:    
    ModuleAddr moduleAddr;
    bool initDone;
    vector<Entry> dynsymEntries;
    vector<Entry> reladynEntries; 
    vector<Entry> relapltEntries;
   
};