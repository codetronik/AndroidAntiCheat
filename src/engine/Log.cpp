#include "Log.h"

AsyncLog* volatile AsyncLog::_instance;
mutex AsyncLog::_mutex;

AsyncLog* volatile AsyncLog::instance()
{
	if (_instance == nullptr)
	{
		lock_guard<mutex> lock(_mutex);
		if (_instance == nullptr)
		{
			_instance = new AsyncLog();

		}

	}
	return _instance;
}

void AsyncLog::Log(int type, const char* str, ...)
{
	char buffer[1024] = { 0, };
	va_list args;

	va_start(args, str);
	vsprintf(buffer, str, args);
	va_end(args);
	string t(buffer);
	q.push(t);
	logType.push(type);
}

void AsyncLog::Thread()
{
	while (true)
	{
		usleep(500);

		if (false == q.empty())
		{
			string a = q.front();
			int type = logType.front();
			__android_log_print(type, "ANTI", a.c_str());
			q.pop();
			logType.pop();
		}
	}
}