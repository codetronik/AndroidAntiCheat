#pragma once
#include <android/log.h>
#include <queue>
#include <unistd.h>
#include <mutex>

#define LOGE(...)   AsyncLog::instance()->Log(ANDROID_LOG_ERROR, __VA_ARGS__)
#define LOG(...)    AsyncLog::instance()->Log(ANDROID_LOG_INFO, __VA_ARGS__)

using namespace std;

// 로그 함수 내부 버그로 인해 짧은 시간 내 대량의 로그 출력이 안되므로 sleep을 주어 비동기로 출력해야함
class AsyncLog
{
private:
	AsyncLog() { }
	AsyncLog(const AsyncLog& other);
	~AsyncLog() { }
public:
	static AsyncLog* volatile instance();
	
	void Log(int type, const char* str, ...);
	void Thread();

private:
	queue<string> q;
	queue<int> logType;
	static AsyncLog* volatile _instance;
	static mutex _mutex;
};

