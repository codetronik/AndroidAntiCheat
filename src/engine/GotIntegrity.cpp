#include "GotIntegrity.h"

GotIntegrity::GotIntegrity(shared_ptr<MemModule> targetMod)
{
	this->targetMod = targetMod;

}
bool GotIntegrity::AddCheckFuncion(const char* func, shared_ptr<MemModule> checkMod)
{
	if (false == checkMod->Init())
	{
		LOG("GotIntegrity::AddCheckFuncion() 1");
		return false;
	}
	optional<ModuleAddr> checkModAddr = checkMod->GetModuleAddr();
	
	if (false == checkModAddr.has_value())
	{
		LOG("GotIntegrity::AddCheckFuncion() 2");
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
		LOG("GotIntegrity::CheckIntegrity() 1");
		return false;
	}
	// .got.plt ������ �ִ� ��츦 �켱�Ѵ�.
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
		// �Լ� �̸��� �˻��Ѵ�.
		auto pos = find_if(checkList.begin(), checkList.end(), [&sym](const CheckFunc& x) { return (0 == strcmp(x.funcName, (const char*)sym)); });
		if (pos != checkList.end())
		{
			
			// üũ����Ʈ�� �Լ��� ���� �������� ����
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
