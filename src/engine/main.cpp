#include <pthread.h>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include "MemModule.h"
#include "GotIntegrity.h"
#include "HookTest.h"
#include "InlineStatus.h"
//#include "Log.h"
#include "MyApi.h"
#include "MemoryScan.h"
#include "SectionIntegrity.h"
#include <thread>
using namespace std;

//AsyncLog log;

string GetSystemLibraryPath()
{
	char sdk_ver[10] = { 0, };
	if (__system_property_get("ro.build.version.sdk", sdk_ver))
	{
		if (30 <= atoi(sdk_ver)) // Android 11 ++
		{
			return "/apex/com.android.runtime/lib64/bionic/";
		}
		else
		{
			return "/system/lib64/";
		}
	}

	return "";
}

class Test
{
public:
	void SetThisModuleBaseAddress(intptr_t thisModAddr)
	{
		this->thisModAddr = thisModAddr;
	}
	bool ModInit()
	{  
		// 자주 쓰는 모듈 추가
		// shared_ptr는 내부에서 alloc을 2번씩 하는 단점이 있음
		mod_libc = make_shared<MemModule>(GetSystemLibraryPath() + "libc.so");
		mod_libdl = make_shared<MemModule>(GetSystemLibraryPath() + "libdl.so");
		mod_sample = make_shared<MemModule>("/data/local/tmp/libsample.so");
	 
		if (false == mod_libdl->Init() || false == mod_libc->Init() || false == mod_sample->Init())
		{
			LOGE("module initialization has failed.");
			return false;
		}

		return true;
	} 
	
	void CheckSectionIntegrity()
	{
		CheckSectionIntegrity(mod_libc);
		CheckSectionIntegrity(mod_sample);
	}
	void CheckGotPltIntegrity()
	{
		GotIntegrity gi(mod_sample); // Set a target to protect.

		// Add export functions to be checked
		gi.AddCheckFuncion("fopen", mod_libc);
		gi.AddCheckFuncion("open", mod_libc);
		gi.AddCheckFuncion("read", mod_libc);
		gi.AddCheckFuncion("stat", mod_libc);
		gi.AddCheckFuncion("access", mod_libc);
		gi.AddCheckFuncion("mprotect", mod_libc);
		gi.AddCheckFuncion("mmap", mod_libc);
		gi.AddCheckFuncion("dlopen", mod_libdl);

		if (gi.IsHooked())
		{
			LOG("GOT is corrupted");
		}
		else
		{
			LOG("GOT is healthy");
		}
	}
	void CheckInlineHook()
	{
		// This method can check inline hooks at a low cost.
		vector<InlineStatus> is;
		is.push_back(InlineStatus((intptr_t)fopen));
		is.push_back(InlineStatus((intptr_t)access));
		is.push_back(InlineStatus((intptr_t)open));
		is.push_back(InlineStatus((intptr_t)stat));
		is.push_back(InlineStatus((intptr_t)dlopen));
		for (auto& i : is)
		{
			LOG("InlineHook Check! %lx %d", (intptr_t)i, i.IsHooked());
		}
	}
	void ScanMemoryHack()
	{
		/* frida-agent-64.so
		.text:00000000007D31FC 00 0A 80 52                                                     MOV             W0, #0x50
		.text:00000000007D3200 02 11 80 52                                                     MOV             W2, #0x88
		.text:00000000007D3204 04 17 80 52                                                     MOV             W4, #0xB8
		.text:00000000007D3208 E6 03 1F 2A                                                     MOV             W6, WZR
		*/
		unsigned char p1[] = { 0x00, 0x0A, 0x80, 0x52, 0x02, 0x11, 0x80, 0x52, 0x04, 0x17, 0x80, 0x52, 0xE6, 0x03, 0x1F, 0x2A };
		
		/* vm app : https://play.google.com/store/apps/details?id=com.excelliance.multiaccounts
		   suspicous malware : libkxqpplatform.so

		LOAD:000000000013B1C4 60 10 08 12                             AND             W0, W3, #0x1F000000
		LOAD:000000000013B1C8 1F 00 02 6B                             CMP             W0, W2
		LOAD:000000000013B1CC A0 0C 00 54                             B.EQ            loc_13B360
		LOAD:000000000013B1D0 60 10 06 12                             AND             W0, W3, #0x7C000000
		LOAD:000000000013B1D4 1F 00 0B 6B                             CMP             W0, W11
		*/
		unsigned char p2[] = { 0x60, 0x10, 0x08, 0x12, 0x1F, 0x00, 0x02, 0x6B, 0xA0, 0x0C, 0x00, 0x54, 0x60, 0x10, 0x06, 0x12, 0x1F, 0x00, 0x0B, 0x6B };

		MemoryScan ms(thisModAddr);
		ms.GetProcessMemoryRegions();
		ms.AddHackPattern(p1, sizeof(p1), PROT_EXEC, (char*)"frida-agent");
		ms.AddHackPattern(p2, sizeof(p2), PROT_EXEC, (char*)"multiple accounts");
		if (true == ms.IsHackFound())
		{
			LOG("HACK is found");
		}
		else
		{
			LOG("HACK isn't found");
		}
	}

private:
	void CheckSectionIntegrity(shared_ptr<MemModule> mod)
	{
		SectionIntegrity si(mod);
		if (false == si.Init())
		{
			return;
		}

		vector<string> sec;
		sec.push_back(".text");
		sec.push_back(".rodata");
		sec.push_back(".plt");
		for (auto& i : sec)
		{
			if (true == si.IsModified(i))
			{
				LOG("[%s] %s is corrupted", i.c_str(), mod_libc->path.c_str());
			}
			else
			{
				LOG("[%s] %s is healthy", i.c_str(), mod_libc->path.c_str());
			}
		}
	}
private:
	shared_ptr<MemModule> mod_libc;
	shared_ptr<MemModule> mod_libdl; 
	shared_ptr<MemModule> mod_sample;
	intptr_t thisModAddr;

};

void __attribute__((constructor)) library_init()
{
	LOG("================> Android Anti-cheat Engine Start!!");

	/*
	thread t([] {
		AsyncLog::instance()->Thread();
		});
	*/
	thread t(bind(&AsyncLog::Thread, AsyncLog::instance()));

	void *handle = dlopen("/data/local/tmp/libsample.so", RTLD_NOW);

	typedef void (*test)();
	test test_func = (test)dlsym(handle, "test");

	InlineHookTest();
	GotPltHookTest();
	DataModifyTest();
	if (test_func != nullptr)
	{
		test_func();
	}

	// get the base address of this module.
	Dl_info dl_info = { 0, };
	dladdr((void*)library_init, &dl_info);
		
	Test m;
	m.SetThisModuleBaseAddress((intptr_t)dl_info.dli_fbase);
	m.ModInit();
	m.CheckGotPltIntegrity();
	m.CheckInlineHook();
	m.ScanMemoryHack();
	m.CheckSectionIntegrity();

	LOG("Done.");
	t.detach();
}