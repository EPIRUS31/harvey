#include "kernel/include.h"
#include "kernel/klog.h"
#include "kernel/config.h"
#include "kernel/skCrypter.h"
#include "kernel/kport.h"
#include "kernel/kutils.h"
#include "DeviceIoControl.h"

namespace gpuenergydriver
{
    PDEVICE_OBJECT pDeviceObject = nullptr;
    PKLDR_DATA_TABLE_ENTRY Entry = nullptr;
    PVOID pBaseAddress = nullptr;

    inline NTSTATUS Initialize()
    {
        pDeviceObject = kutil::pGetDeviceObject(X(L"\\Device\\gpuenergydrv"));
        if (!pDeviceObject)
        {
            return STATUS_INVALID_ADDRESS;
        }

        Entry = kutil::pGetModuleInfo(X(L"gpuenergydrv.sys"));
        if (!Entry)
        {
            return STATUS_INVALID_ACCOUNT_NAME;
        }

        pBaseAddress = Entry->DllBase;
        if (!pBaseAddress)
        {
            return STATUS_INVALID_ACCOUNT_NAME;
        }

        return STATUS_SUCCESS;
    }
}

EXTERN_C VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);
    Klog(X("bye bye..."));
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    kport::Kernel();

    auto devName = X(L"\\Device\\gpuenergydrv");
    auto symName = X(L"\\DosDevices\\gpuenergydrv");
    auto custSymName = X(L"\\DosDevices\\sigmadriver");

    RtlInitUnicodeString(&KGLOBAL::DeviceName, devName.decrypt());
    RtlInitUnicodeString(&KGLOBAL::SymbolicName, symName.decrypt());
    RtlInitUnicodeString(&KGLOBAL::CustomSymbolicName, custSymName.decrypt());

    if (KGLOBAL::BlockDebugging)
    {
        if (kutil::IsKernelDebuggingEnable())
        {
            Klog(X("woah bro debugger really?? | I cant load!!!!"));
            return STATUS_DEBUGGER_INACTIVE;
        }
    }

    KGLOBAL::ManualMapped = (pDriverObject == nullptr);
    if (KGLOBAL::ManualMapped)
    {
        Klog(X("woah bro we are manual mapped"));
    }
    else
    {
        Klog(X("bro really 'sc start driver' ud 2030"));
        pDriverObject->DriverUnload = DriverUnload;
    }

    NTSTATUS status = gpuenergydriver::Initialize();
    if (!NT_SUCCESS(status))
    {
        Klog(X("failed to get setup vuln with status: 0x%X"), status);
        return status;
    }

    Klog(X("harvey1337 is based"));

    PVOID Address = _InterlockedExchangePointer(
        (VOID**)&gpuenergydriver::pDeviceObject->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL],
        (PVOID)deviceiocontrol::IO_IRP_MJ_DEVICE_CONTROL
    );
    if (!Address)
    {
        Klog(X("failed to init hook"));
        return STATUS_UNSUCCESSFUL;
    }

    Klog(X("hook initialized #mainframe"));

    status = IoCreateSymbolicLink(&KGLOBAL::CustomSymbolicName, &KGLOBAL::DeviceName);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
  
    Klog(X("leet link created #based"));
    

    Klog(X("bro driver was success"));
    return STATUS_SUCCESS;
}