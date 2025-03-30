#ifndef DEVICEIOCONTROL_H
#define DEVICEIOCONTROL_H

#include "kernel/include.h"
#include "kernel/klog.h"
#include "kernel/config.h"
#include "kernel/skCrypter.h"
#include "kernel/kport.h"
#include "kernel/kutils.h"

struct SystemRequest
{
    PVOID Address;
    PVOID Buffer;
    SIZE_T BufferSize;

    INT Process; // pid

    enum _CALL
    {
        read,
        write
    }CALL;
};

NTSTATUS ReadPhysicalMemoryWrapper(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesRead);
NTSTATUS WritePhysicalMemoryWrapper(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesWrote);
UINT64 TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress);
UINT64 BruteForceDTB();

namespace target
{
    extern PEPROCESS pTarget;
    extern UINT64 BaseSectionAddress;
    extern UINT64 DirectoryTableBase;
    extern int RequestCount;

    NTSTATUS ReadVirtualMemory(SystemRequest* Request);
    NTSTATUS WriteVirtualMemory(SystemRequest* Request);
}

namespace deviceiocontrol
{
    const ULONG DRIVER_CALL = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS);

    NTSTATUS IO_IRP_MJ_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
    NTSTATUS IO_IRP_MJ_CREATE(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
    NTSTATUS IO_IRP_MJ_CLOSE(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
}

#endif // DEVICEIOCONTROL_H