#include "Module.h"


Module::Module(string path)
{
	this->initDone = false;
	this->path = path;	
	// ����ü�� memset���� �ʱ�ȭ ���� -> ����ü ���� string������ �����ϴ� ���� ���� �� ����
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


// ���� ���� �ְų� ���ų�
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
	// .got ����

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

		// .rela.plt�� ���� �ִ��� ã�ƺ���.
		auto posPlt = find_if(relapltEntries.begin(), relapltEntries.end(), [&value](const Entry& x) { return x.funcAddr == value; });
		if (posPlt != relapltEntries.end())
		{
			//LOG("push --> %p %p [.relaplt]%s", &entry[i], entry[i], posPlt->funcName);
			gotEntries.push_back({ posPlt->funcName, (intptr_t)&entry[i], entry[i], false });
		}

		// .rela.dyn�� ���� �ִ��� ã�ƺ���.
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
	// .got.plt ����
	LOG("Module::GetGotPltEntries()");
	GetRelaPltEntries();

	vector<GotEntry> gotPltEntries;

	// .got.plt & .got �Ѵ� ������ .got.plt�� ����
	// .got�ϳ��� �ִ� ���� .got���� plt �� ���Ե�
	intptr_t* entry = nullptr;
	int entryCnt = 0;


	entry = (intptr_t*)moduleAddr.gotPltSectionStartAddr;
	entryCnt = (moduleAddr.gotPltSectionEndAddr - moduleAddr.gotPltSectionStartAddr) / sizeof(intptr_t);
	
	for (int i = 0; i < entryCnt; i++)
	{
		intptr_t value = entry[i];

		// .rela.plt�� ���� �ִ��� ã�ƺ���.
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
	// .rela.plt ������ ���� .got�� ����Ų��.

	int relapltEntryCnt = (moduleAddr.relapltSectionEndAddr - moduleAddr.relapltSectionStartAddr) / sizeof(Elf64_Rela);

	// ��Ʈ�� 1���� ����Ŵ
	Elf64_Rela* relapltEntry = (Elf64_Rela*)moduleAddr.relapltSectionStartAddr;

	for (int i = 0; i < relapltEntryCnt; i++)
	{
		intptr_t gotAddress = moduleAddr.startAddr + relapltEntry[i].r_offset;
		intptr_t gotValue = *(intptr_t*)gotAddress;
		
		
		// �� ������ entry���� type�� ���� R_AARCH64_JUMP_SLOT (1026) �̴�.
		//LOG("type %d", ELF64_R_TYPE(relapltEntry[i].r_info));
		
		// �ɺ�(�Լ��̸�) ���ϱ�		
		char* funcName = nullptr;
		
		// ��Ʈ���� �ɺ� ������ �ִٸ�
		if (ELF64_R_SYM(relapltEntry[i].r_info) != 0)
		{
			Elf64_Sym* dynsymEntry = (Elf64_Sym*)moduleAddr.dynsymSectionStartAddr;
			dynsymEntry += ELF64_R_SYM(relapltEntry[i].r_info); // .dynsym ���� �ּҿ� symbol offset��ŭ ���ؼ� �ش� entry�� ����Ű�� ��
			funcName = (char*)moduleAddr.dynstrSectionStartAddr + dynsymEntry->st_name; // .dynstr ���� �ּҿ� offset�� ����	
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
	// .dynsym �� .got�� �����Ͽ� �ش� ��⿡�� ����ϴ� ��� �Լ����� ��� �ִ�.
	// fopen, memcpy���� �Լ����� ������ ���� 0�̹Ƿ� .got ���� Ȱ���� �� ����. 

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

	// .rela.dyn ���� �� �Ϻδ� .got�� ����Ű��, symbol ������ ���� ���� ��ټ��̹Ƿ�, .dynsym���� �������� ���Ͽ� ��ġ�ϸ� �Լ����� ä���־�� �Ѵ�.

	int reladynEntryCnt = (moduleAddr.reladynSectionEndAddr - moduleAddr.reladynSectionStartAddr) / sizeof(Elf64_Rela);

	// ��Ʈ�� 1���� ����Ŵ
	Elf64_Rela* reladynEntry = (Elf64_Rela*)moduleAddr.reladynSectionStartAddr;

	for (int i = 0; i < reladynEntryCnt; i++)
	{
		intptr_t address = moduleAddr.startAddr + reladynEntry[i].r_offset;
		intptr_t value = *(intptr_t*)address;

		// �ɺ�(�Լ��̸�) ���ϱ�
		char* funcName = nullptr;

		// ��Ʈ���� �ɺ� ������ �ִٸ�
		if (ELF64_R_SYM(reladynEntry[i].r_info) != 0)
		{
			Elf64_Sym* dynsymEntry = (Elf64_Sym*)moduleAddr.dynsymSectionStartAddr;
			dynsymEntry += ELF64_R_SYM(reladynEntry[i].r_info); // .dynsym ���� �ּҿ� symbol offset��ŭ ���ؼ� �ش� entry�� ����Ű�� ��
			funcName = (char*)moduleAddr.dynstrSectionStartAddr + dynsymEntry->st_name; // .dynstr ���� �ּҿ� offset�� ����
			//LOG("yes symbol %s", funcName);
		}
		else // �ɺ� ������ ���ٸ� .dynsym�� ��Ʈ���� �ּҰ� ��ġ�ϸ� �����ؿ´�.
		{
			auto pos = find_if(dynsymEntries.begin(), dynsymEntries.end(), [&value](const Entry& x) { return x.funcAddr == value; });
			if (pos != dynsymEntries.end())
			{
				funcName = pos->funcName;
				//LOG("[dynsym] yes symbol %s", funcName);

			}
			else // ���ٸ� ���� �����̴�. (�� ��� .data.rel.ro / .data ���� ������ ��Ʈ���� ����Ŵ)
			{
				funcName = (char*)"global";
				//LOG("no.. %s", funcName);
			}
		}
		reladynEntries.push_back({ funcName, value });

	}


}
