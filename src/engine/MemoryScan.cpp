#include "MemoryScan.h"

MemoryScan::MemoryScan(intptr_t thisModAddr)
{
	this->thisModAddr = thisModAddr;
}
MemoryScan::~MemoryScan()
{

}
void MemoryScan::AddHackPattern(unsigned char* pattern, int size, int perms, char* name)
{
	string s;
	s.assign((char*)pattern, size);
	hackPattern.push_back({ s, perms, name });
}

// boyermoore search algorithm
int MemoryScan::MemorySearch(unsigned char* text, int textSize, unsigned char* pattern, int patternSize)
{
	const char WILDCARD = 0xF9;
	
	int BCT[256] = { -1, };
	int Suffix[512] = { 0, };
	int GST[512] = { 0, };

	for (int j = 0; j < patternSize; j++)
	{
		BCT[pattern[j]] = j;
	}	

	/*  Case 1 */
	int psize_cnt = patternSize;
	int psize_cnt_plus = patternSize + 1;

	Suffix[psize_cnt] = patternSize + 1;

	while (psize_cnt > 0)
	{
		while (psize_cnt_plus <= patternSize && pattern[psize_cnt - 1] != pattern[psize_cnt_plus - 1])
		{
			if (GST[psize_cnt_plus] == 0)
			{
				GST[psize_cnt_plus] = psize_cnt_plus - psize_cnt;
			}

			psize_cnt_plus = Suffix[psize_cnt_plus];
		}

		psize_cnt--;
		psize_cnt_plus--;

		Suffix[psize_cnt] = psize_cnt_plus;
	}

	/*  Case 2 */
	psize_cnt_plus = Suffix[0];

	for (int i = 0; i <= patternSize; i++)
	{
		if (GST[i] == 0)
		{
			GST[i] = psize_cnt_plus;
		}
		if (i == psize_cnt_plus)
		{
			psize_cnt_plus = Suffix[psize_cnt_plus];
		}
	}

	int i = 0;

	while (i <= textSize - patternSize)
	{
		int pattern_found_count = patternSize - 1;

		while ((pattern_found_count >= 0 && (pattern[pattern_found_count] == text[i + pattern_found_count])) || pattern[pattern_found_count] == WILDCARD)
		{
			pattern_found_count--;
		}

		// found!
		if (pattern_found_count < 0)
		{			
			return i;
		}

		int move = 0;

		//text를 WILDCARD로 바꾼후 이동한후 다시 원래 값으로 바꿈.
		if (GST[pattern_found_count + 1] > pattern_found_count - BCT[text[i + pattern_found_count]])
		{
			move = GST[pattern_found_count + 1];
		}
		else
		{
			move = pattern_found_count - BCT[text[i + pattern_found_count]];
		}


		if (move == patternSize)
		{
			unsigned char temp = text[i + pattern_found_count];
			text[i + pattern_found_count] = WILDCARD;
			int itemp = i;

			if (GST[pattern_found_count + 1] > pattern_found_count - BCT[text[i + pattern_found_count]])
			{
				i += GST[pattern_found_count + 1];
			}
			else
			{
				i += pattern_found_count - BCT[text[i + pattern_found_count]];
			}

			text[itemp + pattern_found_count] = temp;
		}
		else
		{
			i += move;
		}
	}

	return -1;
}
bool MemoryScan::IsHackFound()
{
	bool ret = false;

	vector<unsigned char> region;
	for (auto& i : memRegions)
	{
		// vector사용 시 컴파일 옵션 -O3 필수. -O0으로 하면 resize()에서 겁나 느림.
		region.resize(i.endAddr - i.startAddr);
		memcpy(region.data(), (void*)i.startAddr, i.endAddr - i.startAddr);

		for (auto& j : hackPattern)
		{
			// 예를들어 코드 영역에만 존재하는 패턴을 PROT_READ | PROT_WRITE 속성의 메모리에서 검색할 필요가 없음
			if (i.perms & j.existPerms)
			{
				int off = MemorySearch(region.data(), i.endAddr - i.startAddr, (unsigned char*)j.hexbytes.data(), j.hexbytes.size());
				if (off > -1)
				{
					LOG("(%lx) %lx %d %s", i.startAddr, i.endAddr - i.startAddr, i.perms, i.path.c_str());
					LOG("detect offset %lx %s", i.startAddr + off, j.name.c_str());
					ret = true;
				}

			}
		}
	}
	return ret;
}

void MemoryScan::GetProcessMemoryRegions()
{
	memRegions.empty();
	MyApi myApi;
	int fd = myApi.open("/proc/self/maps", O_RDONLY, 0);
	if (fd < 0)
	{
		LOGE("GetProcessMemoryRegions() 1");
		return;
	}

	string buffer = read_string(fd); // 파일 버퍼
	close(fd);

	string oneLine = "";
	stringstream ss(buffer);


	// \n 으로 토큰 구분
	while (getline(ss, oneLine, '\n'))
	{
		intptr_t startAddr;
		intptr_t endAddr;

		char perms[6] = { 0, };
		char temp1[30] = { 0, };
		char temp2[30] = { 0, };
		char temp3[30] = { 0, };
		char path[255] = { 0, };

		sscanf(oneLine.c_str(), "%lx-%lx %s %s %s %s %s", &startAddr, &endAddr, perms, temp1, temp2, temp3, path);
		
		if (startAddr == thisModAddr)
		{
			//LOG("this mod skip");
			continue;
		}
		int nperms = 0;
		if (strstr(perms, "r"))
		{
			nperms = nperms + PROT_READ;
		}
		if (strstr(perms, "w"))
		{
			nperms = nperms + PROT_WRITE;
		}
		if (strstr(perms, "x"))
		{
			nperms = nperms + PROT_EXEC;
		}
		//sleep(0.2); // 한 번에 대량의 LOG를 출력하면 중간에 로그가 짤림. sleep을 주어야 함 

		// 읽기 권한 없으면 거름
		if (!(nperms & PROT_READ))
		{
			//LOG("pass %s", oneLine.c_str());			
			continue;
		}

		if (path[0] == '[') // [anon:.bss] 같은거 거름
		{
			//LOG("pass %s", oneLine.c_str());			
			continue;
		}
		else if (strstr(path, ":")) // /dev/__properties__/u:object_r:debug_prop:s0 같은거 거름
		{
			//LOG("pass %s", oneLine.c_str());				
			continue;
		}

		MemoryRegionInfo memInfo = { 0, };
		memInfo.path = string(path);
		memInfo.startAddr = startAddr;
		memInfo.endAddr = endAddr;
		memInfo.perms = nperms;
		memRegions.push_back(memInfo);
		//LOG("%s %s perms~ %d", oneLine.c_str(), path, memInfo.perms);

	}

}