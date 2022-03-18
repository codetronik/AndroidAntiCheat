#include "Util.h"
#define page_size                0x1000
#define page_align(n)            align_up((uintptr_t)n, page_size)
#define align_up(x, n)           (((x) + ((n) - 1)) & ~((n) - 1))
#define align_down(x, n)         ((x) & -(n))

static size_t get_size(void* p, size_t n)
{  
    size_t size = 0;
    if (page_align((uintptr_t)p + n) != page_align((uintptr_t)p))
    {
        size = page_align(n) + page_size;
    }
    else
    {
        size = page_align(n);
    }
    return size;
}
int make_rwx(void* p, size_t n)
{
    intptr_t start_addr_align = align_down((uintptr_t)p, page_size);
    size_t size = get_size(p, n);
    MyApi myApi;
    return myApi.mprotect((void*)start_addr_align, size, PROT_READ | PROT_WRITE | PROT_EXEC);
}
int make_rw(void* p, size_t n)
{
    intptr_t start_addr_align = align_down((uintptr_t)p, page_size);
    size_t size = get_size(p, n);
    MyApi myApi;
    return myApi.mprotect((void*)start_addr_align, size, PROT_READ | PROT_WRITE);
}
string read_string(int fd)
{
    string buffer = ""; // ���� ����
    MyApi myApi;

    while (true)
    {
        // �ѹ��� 4000����Ʈ �̻� �� ������ �� ����
        char temp[3000] = { 0, };

        int size = myApi.read(fd, temp, sizeof(temp) - 1);
        if (size <= 0)
        {
            break;
        }
        string temp2(temp);
        buffer = buffer + temp2;

    }

    return buffer;
}

