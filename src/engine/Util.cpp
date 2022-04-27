#include "Util.h"

#define PAGE_ALIGN(x, n)           (((x) + ((n) - 1)) & ~((n) - 1))

static int make(void* p, size_t n, int perms)
{
	intptr_t start_addr_align = (intptr_t)p & PAGE_MASK;
	intptr_t end_addr_align = PAGE_ALIGN((intptr_t)p + n, PAGE_SIZE);

	int page_size = end_addr_align - start_addr_align;

	MyApi myApi;
	return myApi.mprotect((void*)start_addr_align, page_size, perms);
}

int make_rwx(void* p, size_t n)
{
	return make(p, n, PROT_READ | PROT_WRITE | PROT_EXEC);
}

int make_rw(void* p, size_t n)
{
	return make(p, n, PROT_READ | PROT_WRITE);
}

int make_r(void* p, size_t n)
{
	return make(p, n, PROT_READ);
}

string get_self_maps()
{
	MyApi myApi;
	int fd = myApi.open("/proc/self/maps", 0/*O_RDONLY*/, 0);

	string buffer = ""; // 파일 버퍼

	while (true)
	{
		// 한번에 4000바이트 이상 안 읽히는 것 같음
		char temp[3000] = { 0, };

		int size = myApi.read(fd, temp, sizeof(temp) - 1);
		if (size <= 0)
		{
			break;
		}
		string temp2(temp);
		buffer = buffer + temp2;
	}
	
	close(fd);	

	return buffer;
}