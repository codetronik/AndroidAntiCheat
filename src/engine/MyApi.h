#pragma once
#include <sys/syscall.h>
#include <unistd.h>

class MyApi
{
public:
	int open(const void* path, int flags, int mode);
	int stat(const char* path, struct stat* buf);
	ssize_t read(int fd, void* buf, size_t count);
	void* mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset);
	int munmap(void* addr, size_t size);
	int mprotect(void* addr, size_t size, int prot);
};