#include "FileModule.h"


FileModule::FileModule(string path) : Module(path)
{
}
FileModule::~FileModule()
{
}

bool FileModule::Init()
{
	if (initDone == false)
	{
		if (!GetSectionAddr())
		{
			return false;
		}
	}
	initDone = true;
	return true;
}

bool FileModule::GetSectionAddr()
{
	LOG("FileModule::GetSectionAddr()");
	MyApi m;
	// for mmap
	int fd = m.open(path.c_str(), O_RDONLY, 0);
	if (fd < 0)
	{
		return false;
	}
	// 파일의 사이즈를 얻는다. 
	struct stat finfo = { 0, };
	m.stat(path.c_str(), &finfo);
	
	// elf 파싱을 위해 메모리에 매핑
	char* memory = (char*)mmap(0, finfo.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
	if (MAP_FAILED == memory)
	{
		LOGE("mmap error");
		return false;
	}
	moduleAddr.startAddr = (intptr_t)memory;
	
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)memory;
	if (!(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 0x45 && ehdr->e_ident[2] == 0x4C && ehdr->e_ident[3] == 0x46))
	{
		LOGE("no elf");
		return false;
	}
	
	Elf64_Shdr* shdr = (Elf64_Shdr*)&memory[(ehdr->e_shstrndx * sizeof(Elf64_Shdr)) + ehdr->e_shoff];


	intptr_t sh_offset = shdr->sh_offset;

	int x = 0;
	for (int i = 0; i < ehdr->e_shnum; i++, x += sizeof(Elf64_Shdr))
	{
		shdr = (Elf64_Shdr*)&memory[ehdr->e_shoff + x];
		char* p = &memory[sh_offset + shdr->sh_name];

		//LOG("%s %llx %llx %llx %llx ", p, moduleAddr.startAddr + shdr->sh_addr, shdr->sh_addr, shdr->sh_offset, shdr->sh_size);

		if (0 == strcmp(p, ".got.plt"))
		{
			moduleAddr.gotPltSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.gotPltSectionEndAddr = moduleAddr.gotPltSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".got"))
		{
			moduleAddr.gotSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.gotSectionEndAddr = moduleAddr.gotSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".text"))
		{
			moduleAddr.codeSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.codeSectionEndAddr = moduleAddr.codeSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".dynsym"))
		{
			moduleAddr.dynsymSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.dynsymSectionEndAddr = moduleAddr.dynsymSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".dynstr"))
		{
			moduleAddr.dynstrSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.dynstrSectionEndAddr = moduleAddr.dynstrSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rela.dyn"))
		{
			moduleAddr.reladynSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.reladynSectionEndAddr = moduleAddr.reladynSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rela.plt"))
		{
			moduleAddr.relapltSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.relapltSectionEndAddr = moduleAddr.relapltSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".rodata"))
		{
			moduleAddr.rodataSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.rodataSectionEndAddr = moduleAddr.rodataSectionStartAddr + shdr->sh_size;
		}
		if (0 == strcmp(p, ".init_array"))
		{
		
		}
		if (0 == strcmp(p, ".data"))
		{

		}
		
	}

	return true;

}