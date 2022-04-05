#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <android/log.h>
#include <fcntl.h>

#define LOG(...)    ((void)__android_log_print(ANDROID_LOG_INFO, "ANTI", __VA_ARGS__))

extern "C"
{
	void test();
	int a();
}

int a()
{
	LOG("123");
	return 55;	
}

void test()
{

	int fd = open("/bin/su", O_RDONLY, 0);
	
	if (fd > -1)
	{
		LOG("(open) /bin/su exist");
	}
	else
	{
		LOG("(open) /bin/su not exist");
	}
	fd = open("/apex/com.android.runtime/lib64/bionic/libc.so", O_RDONLY, 0);
	if (fd > -1)
	{
		LOG("(open) /apex/com.android.runtime/lib64/bionic/libc.so exist");
	}
	else
	{
		LOG("(open) /apex/com.android.runtime/lib64/bionic/libc.so not exist");
	}

	if (access("/bin/su", F_OK) == 0)
	{
		LOG("(access) /bin/su exist");
	}
	else
	{
		LOG("(access) /bin/su not exist");
	}    

	struct stat finfo = { 0, };
	stat("/apex/com.android.runtime/lib64/bionic/libc.so", &finfo);
	LOG("(stat) /apex/com.android.runtime/lib64/bionic/libc.so size %d", finfo.st_size);
	memset(&finfo, 0, sizeof(finfo));
	stat("/bin/su", &finfo);
	LOG("(stat) /bin/su size %d", finfo.st_size);
	
}
