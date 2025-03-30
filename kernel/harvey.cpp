#include "kernel/include.h"
#include "kernel/klog.h"
#include "kernel/config.h"
#include "kernel/skCrypter.h"
#include "kernel/kport.h"
#include "kernel/kutils.h"

#include "DeviceIoControl.h"

EXTERN_C auto DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	Klog(X("bye bye..."));
}

EXTERN_C auto DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);

	if (KGLOBAL::BlockDebugging) {
		if (kutil::IsKernelDebuggingEnable())
		{
			Klog(X("woah bro debugger really?? | I cant load!!!!"));
			return STATUS_DEBUGGER_INACTIVE;
		}
	}

	KGLOBAL::ManualMapped = pDriverObject == nullptr ? true : false;

	if (KGLOBAL::ManualMapped)
	{
		Klog(X("woah bro we are manual mapped"));
	}
	else
	{
		Klog(X("bro really 'sc start driver' ud 2030"));
		pDriverObject->DriverUnload = DriverUnload;
	}

	


	Klog(X("bro driver was success"));
	return STATUS_SUCCESS;
}