#include "kernel.h"
#include <chrono>
#include <iostream>

int main()
{
    if (!kernel.Attach(L"notepad.exe"))
    {
        printf("failed to attach\n");
        std::cin.get();
        return -1;
    }

    printf("attached\n");
    printf("process id : %ld\n", kernel.processHandle);
    printf("kernel id : %p\n", kernel.kernelHandle);



    kernel.Detach();
    printf("detached\n");
    std::cin.get();
    return 0;
}