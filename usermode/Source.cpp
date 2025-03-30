#include "kernel.h"

int main()
{
	if (!kernel.Attach(L"RustClient.exe"))
	{
		std::cout << "failed to attach\n";
		return -1;
	}

	std::cout << "attached\n";
	std::cout << "process id " << kernel.processHandle << "\n";
	std::cout << "driver id " << kernel.kernelHandle << "\n";

	uintptr_t base = kernel.GetModuleBase(L"GameAssembly.dll");

	std::cout << "gameassembly : " << base << "\n";

	char Buffer[256];
	
	for (int i = 0; i < 6000; i++)
	{
		kernel.ReadVirtualMemory(base, &Buffer, sizeof(Buffer));
		std::cout << Buffer[0] << Buffer[1] << "\n";
	}





		
	kernel.Detach();
	std::cin.get();
	return 0;
}