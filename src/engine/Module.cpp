#include "Module.h"


Module::Module(string path)
{
	this->initDone = false;
	this->path = path;	
	// 구조체를 memset으로 초기화 금지 -> 구조체 내에 string같은거 선언하는 순간 터질 수 있음
	this->moduleAddr.codeSectionEndAddr = 0;
	this->moduleAddr.codeSectionStartAddr = 0;
	this->moduleAddr.dynstrSectionEndAddr = 0;
	this->moduleAddr.dynstrSectionStartAddr = 0;
	this->moduleAddr.dynsymSectionEndAddr = 0;
	this->moduleAddr.dynsymSectionStartAddr = 0;
	this->moduleAddr.gotPltSectionEndAddr = 0;
	this->moduleAddr.gotPltSectionStartAddr = 0;
	this->moduleAddr.gotSectionEndAddr = 0;
	this->moduleAddr.gotSectionStartAddr = 0;
	this->moduleAddr.reladynSectionEndAddr = 0;
	this->moduleAddr.reladynSectionStartAddr = 0;
	this->moduleAddr.relapltSectionEndAddr = 0;
	this->moduleAddr.relapltSectionStartAddr = 0;
	this->moduleAddr.rodataSectionEndAddr = 0;
	this->moduleAddr.rodataSectionStartAddr = 0;
	this->moduleAddr.startAddr = 0;
}

Module::~Module()
{
}


// 리턴 값이 있거나 없거나
optional<ModuleAddr> Module::GetModuleAddr()
{
	LOG("Module::GetModuleAddr()");
	if (initDone == false)
	{
		LOG("failed");
		return nullopt;
	}
	return moduleAddr;
}
vector<GotEntry> Module::GetGotEntries()
{
	// .got 섹션

	LOG("Module::GetGotEntries()");
	GetDynSymEntries();
	GetRelaPltEntries();
	GetRelaDynEntries();
	vector<GotEntry> gotEntries;

	intptr_t* entry = nullptr;
	int entryCnt = 0;

	entry = (intptr_t*)moduleAddr.gotSectionStartAddr;
	entryCnt = (moduleAddr.gotSectionEndAddr - moduleAddr.gotSectionStartAddr) / sizeof(intptr_t);
	
	for (int i = 0; i < entryCnt; i++)
	{
		intptr_t value = entry[i];

		// .rela.plt에 값이 있는지 찾아본다.
		auto posPlt = find_if(relapltEntries.begin(), relapltEntries.end(), [&value](const Entry& x) { return x.funcAddr == value; });
		if (posPlt != relapltEntries.end())
		{
			//LOG("push --> %p %p [.relaplt]%s", &entry[i], entry[i], posPlt->funcName);
			gotEntries.push_back({ posPlt->funcName, (intptr_t)&entry[i], entry[i], false });
		}

		// .rela.dyn에 값이 있는지 찾아본다.
		auto posDyn = find_if(reladynEntries.begin(), reladynEntries.end(), [&value](const Entry& x) { return x.funcAddr == value; });
		if (posDyn != reladynEntries.end())
		{
			//LOG("push --> %p %p [.reladyn]%s", &entry[i], entry[i], posDyn->funcName);
			gotEntries.push_back({ posDyn->funcName, (intptr_t)&entry[i], entry[i], false });
		}

		if (posPlt == relapltEntries.end() && posDyn == reladynEntries.end())
		{
		//	LOG("--> %p %p [etc]", &entry[i], entry[i], posDyn->funcName);
		}
	}
	return gotEntries;


}


vector<GotEntry> Module::GetGotPltEntries()
{
	if (moduleAddr.gotPltSectionStartAddr == 0)
	{
		return GetGotEntries();
	}
	// .got.plt 섹션
	LOG("Module::GetGotPltEntries()");
	GetRelaPltEntries();

	vector<GotEntry> gotPltEntries;

	// .got.plt & .got 둘다 있으면 .got.plt만 참조
	// .got하나만 있는 경우는 .got에서 plt 도 포함됨
	intptr_t* entry = nullptr;
	int entryCnt = 0;


	entry = (intptr_t*)moduleAddr.gotPltSectionStartAddr;
	entryCnt = (moduleAddr.gotPltSectionEndAddr - moduleAddr.gotPltSectionStartAddr) / sizeof(intptr_t);
	
	for (int i = 0; i < entryCnt; i++)
	{
		intptr_t value = entry[i];

		// .rela.plt에 값이 있는지 찾아본다.
		auto pos = find_if(relapltEntries.begin(), relapltEntries.end(), [&value](const Entry& x) { return x.funcAddr == value; });
		if (pos != relapltEntries.end())
		{		
			//LOG("push --> %p %p [.relaplt]%s", &entry[i], entry[i], pos->funcName);	
			gotPltEntries.push_back({ pos->funcName, (intptr_t)&entry[i], entry[i], false});
		}
		else
		{
		//	LOG("--> %p %p [etc]", &entry[i], entry[i]);
		}
		
	}
	LOG("gotplt done");
	return gotPltEntries;

}

void Module::GetRelaPltEntries()
{
	LOG("Module::GetRelaPltEntries()");
	// .rela.plt 값들은 전부 .got를 가리킨다.

	int relapltEntryCnt = (moduleAddr.relapltSectionEndAddr - moduleAddr.relapltSectionStartAddr) / sizeof(Elf64_Rela);

	// 엔트리 1개를 가리킴
	Elf64_Rela* relapltEntry = (Elf64_Rela*)moduleAddr.relapltSectionStartAddr;

	for (int i = 0; i < relapltEntryCnt; i++)
	{
		intptr_t gotAddress = moduleAddr.startAddr + relapltEntry[i].r_offset;
		intptr_t gotValue = *(intptr_t*)gotAddress;
		
		
		// 이 섹션의 entry들의 type은 전부 R_AARCH64_JUMP_SLOT (1026) 이다.
		//LOG("type %d", ELF64_R_TYPE(relapltEntry[i].r_info));
		
		// 심볼(함수이름) 구하기		
		char* funcName = nullptr;
		
		// 엔트리에 심볼 정보가 있다면
		if (ELF64_R_SYM(relapltEntry[i].r_info) != 0)
		{
			Elf64_Sym* dynsymEntry = (Elf64_Sym*)moduleAddr.dynsymSectionStartAddr;
			dynsymEntry += ELF64_R_SYM(relapltEntry[i].r_info); // .dynsym 시작 주소에 symbol offset만큼 더해서 해당 entry를 가리키게 함
			funcName = (char*)moduleAddr.dynstrSectionStartAddr + dynsymEntry->st_name; // .dynstr 시작 주소에 offset을 더함	
			//LOG("%s %llx", funcName, gotValue);
			relapltEntries.push_back({ funcName, gotValue });
		}
		else
		{			
		//	LOGE("no symbol !!");			
		}
	}
}

void Module::GetDynSymEntries()
{
	// .dynsym 은 .got을 포함하여 해당 모듈에서 사용하는 모든 함수명을 담고 있다.
	// fopen, memcpy같은 함수들은 오프셋 값이 0이므로 .got 에서 활용할 수 없다. 

	Elf64_Sym* dynsymEntry = (Elf64_Sym*)moduleAddr.dynsymSectionStartAddr;
	int dynsymEntryCnt = (moduleAddr.dynsymSectionEndAddr - moduleAddr.dynsymSectionStartAddr) / sizeof(Elf64_Sym);
	for (int i = 0; i < dynsymEntryCnt; i++)
	{
		char* funcName = (char*)moduleAddr.dynstrSectionStartAddr + dynsymEntry[i].st_name;

		if ((intptr_t)dynsymEntry[i].st_value != 0x0)
		{
			//LOG("%s %p", funcName, dynsymEntry[i].st_value);
			dynsymEntries.push_back({ funcName, moduleAddr.startAddr + (intptr_t)dynsymEntry[i].st_value });
		}
	}
}

void Module::GetRelaDynEntries()
{
	LOG("Module::GetRelaDynEntries()");

	// .rela.dyn 값들 중 일부는 .got를 가리키며, symbol 정보가 없는 것이 대다수이므로, .dynsym에서 오프셋을 비교하여 일치하면 함수명을 채워넣어야 한다.

	int reladynEntryCnt = (moduleAddr.reladynSectionEndAddr - moduleAddr.reladynSectionStartAddr) / sizeof(Elf64_Rela);

	// 엔트리 1개를 가리킴
	Elf64_Rela* reladynEntry = (Elf64_Rela*)moduleAddr.reladynSectionStartAddr;

	for (int i = 0; i < reladynEntryCnt; i++)
	{
		intptr_t address = moduleAddr.startAddr + reladynEntry[i].r_offset;
		intptr_t value = *(intptr_t*)address;

		// 심볼(함수이름) 구하기
		char* funcName = nullptr;

		// 엔트리에 심볼 정보가 있다면
		if (ELF64_R_SYM(reladynEntry[i].r_info) != 0)
		{
			Elf64_Sym* dynsymEntry = (Elf64_Sym*)moduleAddr.dynsymSectionStartAddr;
			dynsymEntry += ELF64_R_SYM(reladynEntry[i].r_info); // .dynsym 시작 주소에 symbol offset만큼 더해서 해당 entry를 가리키게 함
			funcName = (char*)moduleAddr.dynstrSectionStartAddr + dynsymEntry->st_name; // .dynstr 시작 주소에 offset을 더함
			//LOG("yes symbol %s", funcName);
		}
		else // 심볼 정보가 없다면 .dynsym의 엔트리와 주소가 일치하면 복사해온다.
		{
			auto pos = find_if(dynsymEntries.begin(), dynsymEntries.end(), [&value](const Entry& x) { return x.funcAddr == value; });
			if (pos != dynsymEntries.end())
			{
				funcName = pos->funcName;
				//LOG("[dynsym] yes symbol %s", funcName);

			}
			else // 없다면 전역 변수이다. (이 경우 .data.rel.ro / .data 등의 섹션의 엔트리를 가리킴)
			{
				funcName = (char*)"global";
				//LOG("no.. %s", funcName);
			}
		}
		reladynEntries.push_back({ funcName, value });

	}


}
