#include "kernel.h"

int main()
{
	if (!kernel.Attach(L"notepad.exe"))
	{
		std::cout << "failed to attach\n";
		return -1;
	}

	std::cout << "attached\n";
	std::cout << "process id " << kernel.processHandle << "\n";
	std::cout << "driver id " << kernel.kernelHandle << "\n";

	uintptr_t base = kernel.GetModuleBase(L"notepad.exe");

	char Buffer[256];

	
		kernel.ReadVirtualMemory(base, &Buffer, sizeof(Buffer));
		std::cout << Buffer[0] << Buffer[1] << std::endl;
	





		
	kernel.Detach();
	std::cin.get();
	return 0;
}