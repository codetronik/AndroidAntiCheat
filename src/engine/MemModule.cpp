#include "MemModule.h"
#include <sstream>
#include <cstring>
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
	
	MyApi myApi;
	int fd = myApi.open("/proc/self/maps", O_RDONLY, 0);

	string buffer = read_string(fd);
	close(fd);
	
	string oneLine = "";
	stringstream ss(buffer);
	LOG("%s", this->path.c_str());
	
	// \n 으로 토큰 구분
	while (getline(ss, oneLine, '\n'))
	{			
		size_t n = oneLine.find(this->path);
		if (n != string::npos)
		{
			// 발견
	
			sscanf(oneLine.c_str(), "%lx-", &this->moduleAddr.startAddr);
			break;
		}	
	}
	
	
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
		char* p = &memory[sh_offset + shdr->sh_name];

		if (0 == strcmp(p, ".got.plt"))
		{
			moduleAddr.gotPltSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.gotPltSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".got"))
		{
			moduleAddr.gotSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.gotSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".text"))
		{
			moduleAddr.codeSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.codeSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".dynsym"))
		{
			moduleAddr.dynsymSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.dynsymSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".dynstr"))
		{
			moduleAddr.dynstrSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.dynstrSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rela.dyn"))
		{
			moduleAddr.reladynSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.reladynSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rela.plt"))
		{
			moduleAddr.relapltSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.relapltSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rodata"))
		{
			moduleAddr.rodataSectionStartAddr = moduleAddr.startAddr + shdr->sh_addr;
			moduleAddr.rodataSectionEndAddr = moduleAddr.startAddr + shdr->sh_addr + shdr->sh_size;
		}
		x += sizeof(Elf64_Shdr);
	}
	
	return true;

}