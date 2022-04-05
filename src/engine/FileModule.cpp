#include "FileModule.h"

FileModule::FileModule(string path) : Module(path)
{
	executable = false;
}
FileModule::~FileModule()
{
}
void FileModule::SetExecutable()
{
	executable = true;
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
		
	LOG("!! %s", path.c_str());
	
	// elf 파싱을 위해 메모리에 매핑
	Mmap m(this->path);
	char* mmap_memory = (char*)m.Alloc();
	if (MAP_FAILED == mmap_memory)
	{
		LOGE("GetSectionAddr() 1");
		return false;
	}
	// malloc으로 해야 파일 경로가 가려짐
	char* memory = (char*)memalign(0x1000, m.GetAllocSize()); // 계속 메모리에 상주해야 하므로 해제하면 안됨	
	memcpy(memory, mmap_memory, m.GetAllocSize()); // malloc한 메모리에 복사 후, mmap은 해제됨

	// 실행 파일로 설정한 경우
	if (executable == true)
	{		
		make_rwx(memory, m.GetAllocSize()); // file 모듈을 실행해야 하는 경우, 실제 메모리 모듈 권한과 동일하게 주어야 한다.
	}
	else
	{
		make_r(memory, m.GetAllocSize()); // 메모리 비교만 하는 경우
	}

	
	moduleAddr.startAddr = (intptr_t)memory;

	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)memory;
	if (!(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 0x45 && ehdr->e_ident[2] == 0x4C && ehdr->e_ident[3] == 0x46))
	{
		LOGE("GetSectionAddr() 2");
		return false;
	}

	Elf64_Shdr* shdr = (Elf64_Shdr*)&memory[(ehdr->e_shstrndx * sizeof(Elf64_Shdr)) + ehdr->e_shoff];

	intptr_t sh_offset = shdr->sh_offset;
	if (sh_offset == 0)
	{
		LOGE("GetSectionAddr() 3");
		return false;
	}

	for (int i = 0, x = 0; i < ehdr->e_shnum; i++, x += sizeof(Elf64_Shdr))
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
		if (0 == strcmp(p, ".plt"))
		{
			moduleAddr.pltSectionStartAddr = moduleAddr.startAddr + shdr->sh_offset;
			moduleAddr.pltSectionEndAddr = moduleAddr.pltSectionStartAddr + shdr->sh_size;
		}		
	}

	return true;

}