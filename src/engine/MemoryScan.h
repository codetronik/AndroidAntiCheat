#pragma once
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include "MyApi.h"
#include "Log.h"
#include "Util.h"

using namespace std;

struct MemoryRegionInfo
{
	intptr_t startAddr;
	intptr_t endAddr;
	int perms; // PROT_READ | PROT_WRITE | PROT_EXEC
	string path;
};

struct HackPattern
{
	string hexbytes;
	int existPerms; // PROT_READ | PROT_EXEC
	string name;
};


class MemoryScan
{
public:
	MemoryScan(intptr_t thisModAddr);
	~MemoryScan();
	void AddHackPattern(unsigned char* pattern, int size, int perms, char* name);
	bool IsHackFound();
	void GetProcessMemoryRegions();

private:
	int MemorySearch(unsigned char* text, int textSize, unsigned char* pattern, int patternSize);
	vector<MemoryRegionInfo> memRegions;
	vector<HackPattern> hackPattern;
	intptr_t thisModAddr;
};