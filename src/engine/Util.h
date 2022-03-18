#pragma once
#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include "MyApi.h"
using namespace std;
int make_rwx(void* p, size_t n);
int make_rw(void* p, size_t n);
string read_string(int fd);
