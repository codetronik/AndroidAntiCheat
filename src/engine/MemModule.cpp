#include "MemModule.h"

MemModule::MemModule(string path) : Module(path)
{

}
MemModule::~MemModule()
{
}

bool MemModule::Init()
{
	LOG("MemModule::Init()");
	if (initDone == false)
	{
		GetBaseAddress();		
		// 메모리에 해당 모듈이 없음
		if (this->moduleAddr.startAddr == 0)
		{
			LOGE("MemModule::Init() 1");
			return false;
		}
		if (false == GetSectionAddr())
		{
			LOGE("MemModule::Init() 2");
			return false;
		}
	}
	initDone = true;
	return true;
}


void MemModule::GetBaseAddress()
{
	LOG("MemModule::GetBaseAddress()");

	// 해커가 나중에 mmap한 라이브러리인지, 개발자가 dlopen한 라이브러리인지 보장이 안됨 		
	string oneLine = "";
	stringstream ss(get_self_maps());
	LOG("%s", path.c_str());
	bool first = false;
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
			LOG("%s", oneLine.c_str());

			if (first == false)
			{
				moduleAddr.startAddr = temp1;
				first = true;
			}
			
			if (first)
			{
				moduleAddr.endAddr = temp2;
			}
				
		}	
	}
	LOG("start %lx end %lx", moduleAddr.startAddr, moduleAddr.endAddr);
	
}

bool MemModule::GetSectionAddr()
{
	LOG("MemModule::GetSectionAddr()");

	// elf 파싱을 위해 메모리에 매핑
	Mmap m(this->path);
	
	char* memory = (char*)m.Alloc();

	if (MAP_FAILED == memory)
	{
		LOGE("MemModule::GetSectionAddr() 1");
		return false;
	}

	Elf64_Ehdr* ehdr;
	Elf64_Shdr* shdr;
	ehdr = (Elf64_Ehdr*)memory;
	if (!(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 0x45 && ehdr->e_ident[2] == 0x4C && ehdr->e_ident[3] == 0x46))
	{
		LOGE("MemModule::GetSectionAddr() 2");
		return false;
	}

	shdr = (Elf64_Shdr*)&memory[(ehdr->e_shstrndx * sizeof(Elf64_Shdr)) + ehdr->e_shoff];

	intptr_t sh_offset = shdr->sh_offset;

	int x = 0;
	for (int i = 0; i < ehdr->e_shnum; i++)
	{
	
		shdr = (Elf64_Shdr*)&memory[ehdr->e_shoff + x];
		//LOG("type %d flags %d info %d", shdr->sh_type, shdr->sh_flags, shdr->sh_info);
		char* p = &memory[sh_offset + shdr->sh_name];

		if (0 == strcmp(p, ".got.plt"))
		{
			moduleAddr.gotPltSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.gotPltSectionEndAddr = moduleAddr.gotPltSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".got"))
		{
			moduleAddr.gotSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.gotSectionEndAddr = moduleAddr.gotSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".text"))
		{
			moduleAddr.codeSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.codeSectionEndAddr = moduleAddr.codeSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".dynsym"))
		{
			moduleAddr.dynsymSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.dynsymSectionEndAddr = moduleAddr.dynsymSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".dynstr"))
		{
			moduleAddr.dynstrSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.dynstrSectionEndAddr = moduleAddr.dynstrSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rela.dyn"))
		{
			moduleAddr.reladynSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.reladynSectionEndAddr = moduleAddr.reladynSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rela.plt"))
		{
			moduleAddr.relapltSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.relapltSectionEndAddr = moduleAddr.relapltSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rodata"))
		{
			moduleAddr.rodataSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.rodataSectionEndAddr = moduleAddr.rodataSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".plt"))
		{
			moduleAddr.pltSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.pltSectionEndAddr = moduleAddr.pltSectionStartAddr + shdr->sh_size;
		}
		x += sizeof(Elf64_Shdr);
	}
	
	return true;

}