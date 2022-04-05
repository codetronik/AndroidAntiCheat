#pragma once
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include "MyApi.h"
#include "Log.h"
#include "Util.h"

using namespace std;

class Mmap
{
public:
	Mmap(string path);
	virtual ~Mmap();
	void* Alloc();
	size_t GetAllocSize();
private:
	void DoMmap();
	void DoMunmap();
	bool FileOpen();
	void ChangeAllocPermission();
private:
	bool doAlloc;
	void* memory;
	int fd;
	int size;
	string path;
	size_t allocSize;
};
