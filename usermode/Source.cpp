#include "kernel.h"
#include <chrono>
#include <iostream>

int main()
{
    if (!kernel.Attach(L"RustClient.exe"))
    {
        printf("failed to attach\n");
        std::cin.get();
        return -1;
    }

    uintptr_t processbase = kernel.GetModuleBase(L"RustClient.exe"); // should work 

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