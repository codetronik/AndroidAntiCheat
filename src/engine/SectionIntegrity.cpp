#include "SectionIntegrity.h"


SectionIntegrity::SectionIntegrity(shared_ptr<MemModule> targetMod)
{
	this->targetMod = targetMod;

}
SectionIntegrity::~SectionIntegrity()
{


}
bool SectionIntegrity::Init()
{
	LOG("SectionIntegrity::Init() : target %s", targetMod->path.c_str());

	if (false == targetMod->GetModuleAddr().has_value())
	{
		LOGE("SectionIntegrity::Init() 1");
		return false;
	}

	FileModule fm(targetMod->path.c_str());
	if (false == fm.Init())
	{
		LOGE("SectionIntegrity::Init() 2");
		return false;
	}
	fileModAddr = fm.GetModuleAddr();

	if (false == fileModAddr.has_value())
	{
		LOGE("SectionIntegrity::Init() 3");
		return false;
	}

	return true;
}
bool SectionIntegrity::IsModified(string section)
{
	bool ret = false;
	
	if (CheckSectionIntegrity(section))
	{
		ret = true;
	}
	return ret;
}

bool SectionIntegrity::CheckSectionIntegrity(string section)
{
	bool ret = false;
	intptr_t compareStartAddr = 0;
	intptr_t compareEndAddr = 0;
	intptr_t targetStartAddr = 0;
	intptr_t targetEndAddr = 0;
	int compareSize = 0;
	int targetSize = 0;

	// code section
	if (section == ".text")
	{    
		compareStartAddr = fileModAddr->codeSectionStartAddr;
		compareEndAddr = fileModAddr->codeSectionEndAddr;       
		targetStartAddr = targetMod->GetModuleAddr()->codeSectionStartAddr;
		targetEndAddr = targetMod->GetModuleAddr()->codeSectionEndAddr;       
	}
	else if (section == ".plt")
	{
		compareStartAddr = fileModAddr->pltSectionStartAddr;
		compareEndAddr = fileModAddr->pltSectionEndAddr;       
		targetStartAddr = targetMod->GetModuleAddr()->pltSectionStartAddr;
		targetEndAddr = targetMod->GetModuleAddr()->pltSectionEndAddr;     
	}
	// read-only data section
	else if (section == ".rodata")
	{
		compareStartAddr = fileModAddr->rodataSectionStartAddr;
		compareEndAddr = fileModAddr->rodataSectionEndAddr;        
		targetStartAddr = targetMod->GetModuleAddr()->rodataSectionStartAddr;
		targetEndAddr = targetMod->GetModuleAddr()->rodataSectionEndAddr;        
	}
	else
	{

	}

	compareSize = compareEndAddr - compareStartAddr;
	targetSize = targetEndAddr - targetStartAddr;
	if (targetSize != compareSize)
	{
		LOGE("CheckSectionIntegrity 1 %x %x", compareSize, targetSize);
		return false;
	}

	unsigned char* target = (unsigned char*)targetStartAddr;
	unsigned char* compare = (unsigned char*)compareStartAddr;
	for (int i = 0; i < targetSize; i++)
	{
		if (target[i] != compare[i])
		{
			LOG("[Modified] file offset %p mem offset %p offset %x", compare + i, target + i, i);
			ret = true;
		}
	}

	return ret;
}