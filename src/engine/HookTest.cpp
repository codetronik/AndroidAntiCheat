#include "HookTest.h"
#include "InlineHooker.h"
int (*Org_open)(const char* __path, int __flags);
int hook_open(const char* __path, int __flags)
{
	LOG("inlinehook_open %s", __path);
	if (strstr(__path, "/su") || strstr(__path, "magisk"))
	{       
		return -1;
	}
	return Org_open(__path, __flags);
}

FILE*(*Org_fopen)(const char* path, const char* mode);
FILE* hook_fopen(const char* path, const char* mode)
{
	if (strstr(path, "/su") || strstr(path, "magisk"))
	{
		LOG("fopen path - %s", path);
		return 0;
	}

	return Org_fopen(path, mode);
}
int (*Org_access)(const char* __path, int __mode);
int hook_access(const char* __path, int __mode)
{
	if (strstr(__path, "/su") || strstr(__path, "magisk"))
	{
		LOG("access path - %s", __path);
		return -1;
	}
	return Org_access(__path, __mode);
}
int (*Org_stat)(const char* __path, struct stat* __buf);
int hook_stat(const char* __path, struct stat* __buf)
{
	LOG("hook_stat %s", __path);
	if (strstr(__path, "/su") || strstr(__path, "magisk"))
	{      
		return 0;
	}
	return Org_stat(__path, __buf);
}


void InlineHookTest()
{
	A64HookFunction((void*)fopen, (void*)hook_fopen, (void**)&Org_fopen);
	A64HookFunction((void*)open, (void*)hook_open, (void**)&Org_open);
	A64HookFunction((void*)stat, (void*)hook_stat, (void**)&Org_stat);
	A64HookFunction((void*)access, (void*)hook_access, (void**)&Org_access);
}

void GotPltHookTest()
{
	LOG("GotPltHookTest()");

	MemModule mod("/data/local/tmp/libsample.so");

	if (!mod.Init())
	{
		return;
	}

	vector<GotEntry> gotPltEntries = mod.GetGotPltEntries();

	for (auto& i : gotPltEntries)
	{
		intptr_t* abc = (intptr_t*)i.gotAddr;
		
		//hook test
		if (strcmp(i.funcName, "fopen") == 0)
		{                               
			make_rw(abc, 8); // got섹션에 "x"권한을 주면 crash 발생함           
			*abc = (intptr_t)hook_fopen;
   
		}        
		if (strcmp(i.funcName, "open") == 0)
		{            
			make_rw(abc, 8);
			*abc = (intptr_t)hook_open;
		}        
		if (strcmp(i.funcName, "stat") == 0)
		{            
			make_rw(abc, 8);
			*abc = (intptr_t)hook_stat;
		}        
		if (strcmp(i.funcName, "access") == 0)
		{
			make_rw(abc, 8);
			*abc = (intptr_t)hook_access;
		}
	}

	//got hook을 하여도 fopen의 주소는 변화하지 않음
	//LOG("fopen %llx %llx", fopen, &fopen);


}

void DataModifyTest()
{
	MemModule mod("/data/local/tmp/libsample.so");

	if (!mod.Init())
	{
		return;
	}

	char* start = (char*)mod.GetModuleAddr()->startAddr;
	make_rwx(start, 4096);
	for (int i = 0; i < 4096; i++)
	{
		if (0 == strcmp(start + i, "/bin/su"))
		{
			LOG("found");
			strcpy((char*)start + i, "/bin/s1");
		}
	}
}