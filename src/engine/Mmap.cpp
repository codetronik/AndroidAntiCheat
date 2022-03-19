#include "Mmap.h"

Mmap::Mmap(string path)
{
	doAlloc = false;
	memory = nullptr;	
	this->path = path;
	this->fd = -1;
	this->size = 0;
}

Mmap::~Mmap()
{
	DoMunmap();
	
}

void* Mmap::Alloc()
{
	if (!doAlloc)
	{
		if (FileOpen() == false)
		{
			return nullptr;
		}
		DoMmap();
	}
	doAlloc = true;
	return memory;
}

bool Mmap::FileOpen()
{
	MyApi myApi;
	int fd = myApi.open(this->path.c_str(), O_RDONLY, 0);

	if (fd < 0)
	{
		LOGE("Mmap::FileOpen() 1");
		return false;
	}

	this->fd = fd;
	// 파일의 사이즈를 얻는다. 
	struct stat finfo = { 0, };
	int ret = myApi.stat(this->path.c_str(), &finfo);

	if (ret == -1)
	{
		LOGE("Mmap::FileOpen() 2");

		return false;
	}
	this->size = finfo.st_size;

	return true;

}


void Mmap::DoMmap()
{	
	MyApi myApi;
	memory = myApi.mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);	
}

void Mmap::DoMunmap()
{
	if (memory)
	{
		MyApi myApi;
		myApi.munmap(memory, size);
	}
	close(this->fd);
}
