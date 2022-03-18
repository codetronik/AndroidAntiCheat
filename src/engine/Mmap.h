#pragma once
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "MyApi.h"
#include "Log.h"
using namespace std;

class Mmap
{
public:
    Mmap(string path);
    virtual ~Mmap();
    void* Alloc();
private:
    void DoMmap();
    void DoMunmap();
    bool FileOpen();
private:
    bool doAlloc;
    void* memory;
    int fd;
    int size;
    string path;
};