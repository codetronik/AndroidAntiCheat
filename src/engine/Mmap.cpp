#include "Mmap.h"

Mmap::Mmap(string path)
{
	doAlloc = false;
	memory = nullptr;	
	this->path = path;
	this->fd = -1;
	this->size = 0;
	this->allocSize = 0;
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
		ChangeAllocPermission();
		if (0 == allocSize)
		{
			return nullptr;
		}
	}
	doAlloc = true;
	return memory;
}

bool Mmap::FileOpen()
{
	MyApi myApi;
	int fd = myApi.open(path.c_str(), O_RDONLY, 0);

	if (fd < 0)
	{
		LOGE("Mmap::FileOpen() 1");
		return false;
	}

	this->fd = fd;
	// 파일의 사이즈를 얻는다. 
	struct stat finfo = { 0, };
	int ret = myApi.stat(path.c_str(), &finfo);

	if (ret == -1)
	{
		LOGE("Mmap::FileOpen() 2");

		return false;
	}
	this->size = finfo.st_size;

	return true;

}
size_t Mmap::GetAllocSize()
{
	return allocSize;
}

void Mmap::ChangeAllocPermission()
{
	string oneLine = "";
	stringstream ss(get_self_maps());

	// \n 으로 토큰 구분
	while (getline(ss, oneLine, '\n'))
	{
		size_t n = oneLine.find(path);
		if (n != string::npos)
		{
			intptr_t temp1 = 0;
			intptr_t temp2 = 0;
			char perms[5] = { 0, };

			sscanf(oneLine.c_str(), "%lx-%lx %s", &temp1, &temp2, perms);
			
			if (0 == strcmp(perms, "---p"))
			{
				//LOG("=> %s", oneLine.c_str());
				make_rw((void*)temp1, temp2 - temp1);
				allocSize = temp2 - temp1;
				return;
			}
		}
	}
	LOGE("ChangeRegionPermission");
	
}

void Mmap::DoMmap()
{	
	MyApi myApi;
	// maps에서 찾기 쉽게 하기 위해 일부러 권한을 아무것도 안줌
	memory = myApi.mmap(0, size, 0, MAP_PRIVATE, fd, 0);		
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
