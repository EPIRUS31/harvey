#include "kernel.h"
#include <chrono>
#include <iostream>

int main()
{
    if (!kernel.Attach(L"AOCClient-Win64-Shipping.exe"))
    {
        printf("failed to attach\n");
        std::cin.get();
        return -1;
    }

    uintptr_t processbase = 0x7ff7dda20000;

    printf("attached\n");
    printf("process id : %ld\n", kernel.processHandle);
    printf("kernel id : %p\n", kernel.kernelHandle);
    printf("process base : 0x%p\n", (void*)processbase);


    char buffer[26];
    kernel.ReadVirtualMemory(processbase, buffer, sizeof(buffer));
    std::cout << buffer[0] << buffer[1] << "\n";

    kernel.Detach();
    printf("detached\n");
    std::cin.get();
    return 0;
}