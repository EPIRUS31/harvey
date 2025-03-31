#include "kernel.h"
#include <chrono>
#include <iostream>

int main()
{
    if (!kernel.Attach(L"cmd.exe"))
    {
        printf("failed to attach\n");
        std::cin.get();
        return -1;
    }

    printf("attached\n");
    printf("process id : %ld\n", kernel.processHandle);
    printf("kernel id : %p\n", kernel.kernelHandle);

    uintptr_t RustClient_base = kernel.GetModuleBase(L"cmd.exe");
    printf("RustClient base : 0x%p\n", (void*)RustClient_base);

    char Buffer[32];

    if (!kernel.ReadVirtualMemory(RustClient_base, Buffer, sizeof(Buffer)))
    {
        printf("Failed to read memory\n");
        kernel.Detach();
        std::cin.get();
        return -1;
    }

    printf("MZ Header: %c%c\n", Buffer[0], Buffer[1]);



   // Buffer[0] = 'E';
   // Buffer[1] = 'Z';
   // kernel.WriteVirtualMemory(RustClient_base, Buffer, sizeof(Buffer));

    if (!kernel.ReadVirtualMemory(RustClient_base, Buffer, sizeof(Buffer)))
    {
        printf("Failed to read memory\n");
        kernel.Detach();
        std::cin.get();
        return -1;
    }

    printf("MZ Header: %c%c\n", Buffer[0], Buffer[1]);
    
    

    kernel.Detach();
    printf("detached\n");
    std::cin.get();
    return 0;
}