#include "GotIntegrity.h"

GotIntegrity::GotIntegrity(shared_ptr<MemModule> targetMod)
{
	this->targetMod = targetMod;

}
bool GotIntegrity::AddCheckFuncion(const char* func, shared_ptr<MemModule> checkMod)
{
	if (false == checkMod->Init())
	{
		LOGE("GotIntegrity::AddCheckFuncion() 1");
		return false;
	}
	optional<ModuleAddr> checkModAddr = checkMod->GetModuleAddr();
	
	if (false == checkModAddr.has_value())
	{
		LOGE("GotIntegrity::AddCheckFuncion() 2");
		return false;
	}

	checkList.push_back({ func, checkModAddr.value()});
	LOG("[!] checkList add %s %lx", func, checkModAddr.value().startAddr);
	return true;
}


bool GotIntegrity::IsHooked()
{
	bool ret = false;
	LOG("GotIntegrity::CheckIntegrity()");
	
	vector<GotEntry> gotPltEntries;
	
	if (false == targetMod->Init())
	{
		LOGE("GotIntegrity::CheckIntegrity() 1");
		return false;
	}

	// .got.plt 섹션이 있는 경우를 우선한다.
	optional<ModuleAddr> moduleAddr = targetMod->GetModuleAddr();
	
	if (false == moduleAddr.has_value())
	{
		LOGE("GotIntegrity::CheckIntegrity() 2");
		return false;
	}

	if (moduleAddr->gotPltSectionStartAddr != 0)
	{
		gotPltEntries = targetMod->GetGotPltEntries();
	}
	else
	{
		gotPltEntries = targetMod->GetGotEntries();
	}
	
	for (auto& i : gotPltEntries)
	{	
		char* sym = i.funcName;
		// 함수 이름을 검색한다.
		auto pos = find_if(checkList.begin(), checkList.end(), [&sym](const CheckFunc& x) { return (0 == strcmp(x.funcName, (const char*)sym)); });
		if (pos != checkList.end())
		{
			
			// 체크리스트의 함수가 정상 범위인지 조사
			if (pos->modAddr.codeSectionStartAddr <= i.funcAddr && pos->modAddr.codeSectionEndAddr > i.funcAddr)
			{
				LOG("%s is okay %lx", sym, i.funcAddr);
				ret = false;			
			}
			else
			{
				LOG("%s is hooked %lx", sym, i.funcAddr);
				ret = true;
	
			}
		}
	}	
	return ret;
}
