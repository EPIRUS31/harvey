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

NTSTATUS ReadPhysicalMemoryWrapper(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesRead)
{
        if (!targetAddress || !buffer || size == 0)
            return STATUS_INVALID_PARAMETER;
        MM_COPY_ADDRESS copyAddress = { 0 };
        copyAddress.PhysicalAddress.QuadPart = reinterpret_cast<ULONGLONG>(targetAddress);
        return MmCopyMemory(buffer, copyAddress, size, MM_COPY_MEMORY_PHYSICAL, bytesRead);
};

UINT64 TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress) {
    DirectoryTableBase &= ~0xf;

    const UINT64 PageMask = (~0xfull << 8) & 0xfffffffffull;

    virt_addr_t virtualBase;
    virtualBase.value = (void*)VirtualAddress;

    MMPTE pml4Entry{};
    SIZE_T bytesRead;

    if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(PVOID(DirectoryTableBase + 8 * virtualBase.pml4_index), &pml4Entry, 8, &bytesRead)))
        return false;

    if (!pml4Entry.u.Hard.Valid)
        return false;

    MMPTE pdptEntry{};
    if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(PVOID((pml4Entry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pdpt_index), &pdptEntry, 8, &bytesRead)))
        return false;

    if (!pdptEntry.u.Hard.Valid)
        return false;

    MMPTE pdEntry{};
    if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(PVOID((pdptEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pd_index), &pdEntry, 8, &bytesRead)))
        return false;

    if (!pdEntry.u.Hard.Valid)
        return false;

    MMPTE ptEntry{};
    if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(PVOID((pdEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pt_index), &ptEntry, 8, &bytesRead)))
        return false;

    if (!ptEntry.u.Hard.Valid)
        return false;

    return (ptEntry.u.Hard.PageFrameNumber << 12) | (VirtualAddress & 0xfff);
}

namespace target
{
	PEPROCESS pTarget;
	UINT64 DirectoryTableBase;


	NTSTATUS ReadVirtualMemory(SystemRequest* Request);
	NTSTATUS WriteVirtualMemory(SystemRequest* Request);
}

namespace deviceiocontrol
{
	NTSTATUS IO_IRP_MJ_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
	NTSTATUS IO_IRP_MJ_CREATE(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
	NTSTATUS IO_IRP_MJ_CLOSE(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
}

#endif DEVICEIOCONTROL_H