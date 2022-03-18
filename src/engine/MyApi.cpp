#include "MyApi.h"

int MyApi::open(const void* path, int flags, int mode)
{
    long x0 = -100;
    long x1 = (long)path;
    long x2 = (long)flags;
    long x3 = mode;
    long x8 = __NR_openat;
    long ret = 0;

    // �������� ������ �Ⱦ��� ���� �������Ϳ� ������ ����
    __asm volatile ("mov x0, %0": : "r"(x0));
    __asm volatile ("mov x1, %0": : "r"(x1));
    __asm volatile ("mov x2, %0": : "r"(x2));
    __asm volatile ("mov x3, %0": : "r"(x3));
    __asm volatile ("mov x8, %0": : "r"(x8));
    __asm volatile ("svc 0");
    __asm volatile ("mov %0, x0" : "=r"(ret));

    return (int)ret;
    /*
    // ����ν�� �������� ������ �Ⱦ��� ��� ���̳ʸ��� LDR��ɾ ����� �Ⱥ���
    // �������� ������ ���� ����� LDR X0, ����  �̷������� ����
    // �������� ������ c++���� �������� �ȵǹǷ� ��¿ �� ���� c���Ϸ� �ۼ��Ͽ���

    register long x8 __asm__("x8") = (long)__NR_openat;
    register long x0 __asm__("x0") = (long) - 100; // AT_FDCWD
    register long x1 __asm__("x1") = (long)path;
    register long x2 __asm__("x2") = (long)flags;
    register long x3 __asm__("x3") = 0;
    __asm volatile ("svc 0" : "=r"(x0) : "r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3) : "memory", "cc");

    return x0;
    */
}


int MyApi::stat(const char* path, struct stat* buf)
{  
    long x0 = 0xFFFFFF9CLL;
    long x1 = (long)path;
    long x2 = (long)buf;
    long x3 = 0;
    long x8 = __NR3264_fstatat;
    long ret = 0;
  
    __asm volatile ("mov x0, %0": : "r"(x0));
    __asm volatile ("mov x1, %0": : "r"(x1));
    __asm volatile ("mov x2, %0": : "r"(x2));
    __asm volatile ("mov x3, %0": : "r"(x3));
    __asm volatile ("mov x8, %0": : "r"(x8));
    __asm volatile ("svc 0");
    __asm volatile ("mov %0, x0" : "=r"(ret));

    return (int)ret;    
}


ssize_t MyApi::read(int fd, void* buf, size_t count)
{
    
    long x0 = (long)fd;
    long x1 = (long)buf;
    long x2 = (long)count;
    long x8 = __NR_read;
    long ret = 0;

    __asm volatile ("mov x0, %0": : "r"(x0));
    __asm volatile ("mov x1, %0": : "r"(x1));
    __asm volatile ("mov x2, %0": : "r"(x2));  
    __asm volatile ("mov x8, %0": : "r"(x8));
    __asm volatile ("svc 0");
    __asm volatile ("mov %0, x0" : "=r"(ret));

    return (ssize_t)ret;
}
void* MyApi::mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset)
{
    long x0 = (long)addr;
    long x1 = (long)size;
    long x2 = (long)prot;
    long x3 = (long)flags;
    long x4 = (long)fd;
    long x5 = (long)offset;    
    long x8 = __NR_mmap;
    long ret = 0;

    __asm volatile ("mov x0, %0": : "r"(x0));
    __asm volatile ("mov x1, %0": : "r"(x1));
    __asm volatile ("mov x2, %0": : "r"(x2));
    __asm volatile ("mov x3, %0": : "r"(x3));
    __asm volatile ("mov x4, %0": : "r"(x4));
    __asm volatile ("mov x5, %0": : "r"(x5));
    __asm volatile ("mov x8, %0": : "r"(x8));
    __asm volatile ("svc 0");
    __asm volatile ("mov %0, x0" : "=r"(ret));
    return (void*)ret;
}

int MyApi::munmap(void* addr, size_t size)
{
    long x0 = (long)addr;
    long x1 = (long)size;
    long x8 = __NR_munmap;
    long ret = 0;
    __asm volatile ("mov x0, %0": : "r"(x0));
    __asm volatile ("mov x1, %0": : "r"(x1));
    __asm volatile ("mov x8, %0": : "r"(x8));
    __asm volatile ("svc 0");
    __asm volatile ("mov %0, x0" : "=r"(ret));
    return (int)ret;
    
}

int MyApi::mprotect(void* addr, size_t size, int prot)
{
    long x0 = (long)addr;
    long x1 = (long)size;
    long x2 = (long)prot;
    long x8 = __NR_mprotect;
    long ret = 0;

    __asm volatile ("mov x0, %0": : "r"(x0));
    __asm volatile ("mov x1, %0": : "r"(x1));
    __asm volatile ("mov x2, %0": : "r"(x2));
    __asm volatile ("mov x8, %0": : "r"(x8));
    __asm volatile ("svc 0");
    __asm volatile ("mov %0, x0" : "=r"(ret));

    return (int)ret;
}