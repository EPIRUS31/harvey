#include "DeviceIoControl.h"

NTSTATUS ReadPhysicalMemoryWrapper(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesRead)
{
	if (!targetAddress || !buffer || size == 0 || !bytesRead)
		return STATUS_INVALID_PARAMETER;

	MM_COPY_ADDRESS copyAddress = { 0 };
	copyAddress.PhysicalAddress.QuadPart = reinterpret_cast<ULONGLONG>(targetAddress);

	return kport::MmCopyMemory(buffer, copyAddress, size, MM_COPY_MEMORY_PHYSICAL, bytesRead);
}
NTSTATUS WritePhysicalMemoryWrapper(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesWrote)
{
	if (!targetAddress || !buffer || size == 0 || !bytesWrote)
		return STATUS_INVALID_PARAMETER;

	PHYSICAL_ADDRESS physicalAddr = { 0 };
	physicalAddr.QuadPart = (ULONGLONG)targetAddress;

	SIZE_T alignedSize = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	PVOID mappedAddress = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	__try
	{
		mappedAddress = MmMapIoSpaceEx(physicalAddr, alignedSize, PAGE_READWRITE);
		if (!mappedAddress)
		{
			status = STATUS_ACCESS_DENIED;
			__leave;
		}

		RtlCopyMemory(mappedAddress, buffer, size);
		*bytesWrote = size;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
	}

	if (mappedAddress)
		MmUnmapIoSpace(mappedAddress, alignedSize);

	return status;
}
UINT64 TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress)
{
	if (!DirectoryTableBase || !VirtualAddress)
		return 0;

	DirectoryTableBase &= ~0xf;
	virt_addr_t virtualBase;
	virtualBase.value = (void*)target::BaseSectionAddress;
	MMPTE pml4Entry = { 0 };
	SIZE_T bytesRead = 0;

	NTSTATUS status = ReadPhysicalMemoryWrapper(
		(PVOID)(DirectoryTableBase + 8 * virtualBase.pml4_index),
		&pml4Entry,
		sizeof(MMPTE),
		&bytesRead
	);

	if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE) || !pml4Entry.u.Hard.Valid)
		return 0;

	MMPTE pdptEntry = { 0 };
	status = ReadPhysicalMemoryWrapper(
		(PVOID)((pml4Entry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pdpt_index),
		&pdptEntry,
		sizeof(MMPTE),
		&bytesRead
	);

	if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE) || !pdptEntry.u.Hard.Valid)
		return 0;

	MMPTE pdEntry = { 0 };
	status = ReadPhysicalMemoryWrapper(
		(PVOID)((pdptEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pd_index),
		&pdEntry,
		sizeof(MMPTE),
		&bytesRead
	);

	if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE) || !pdEntry.u.Hard.Valid)
		return 0;

	MMPTE ptEntry = { 0 };
	status = ReadPhysicalMemoryWrapper(
		(PVOID)((pdEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pt_index),
		&ptEntry,
		sizeof(MMPTE),
		&bytesRead
	);

	if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE) || !ptEntry.u.Hard.Valid)
		return 0;

	return (ptEntry.u.Hard.PageFrameNumber << 12) | (VirtualAddress & 0xfff);
}
UINT64 BruteForceDTB()
{
	if (!target::BaseSectionAddress)
		return 0;

	virt_addr_t virtualBase;
	virtualBase.value = (void*)target::BaseSectionAddress;
	PPHYSICAL_MEMORY_RANGE physicalMemoryRanges = MmGetPhysicalMemoryRanges();
	if (!physicalMemoryRanges)
		return 0;

	UINT64 foundDTB = 0;
	__try
	{
		for (int i = 0; ; i++)
		{
			PHYSICAL_MEMORY_RANGE range = physicalMemoryRanges[i];
			if (!range.BaseAddress.QuadPart || !range.NumberOfBytes.QuadPart)
				break;

			UINT64 currentPhysical = range.BaseAddress.QuadPart;
			UINT64 rangeEnd = currentPhysical + range.NumberOfBytes.QuadPart;

			for (; currentPhysical < rangeEnd; currentPhysical += 0x1000)
			{
				MMPTE pml4Entry = { 0 };
				SIZE_T bytesRead = 0;
				if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(
					(PVOID)(currentPhysical + 8 * virtualBase.pml4_index),
					&pml4Entry,
					sizeof(MMPTE),
					&bytesRead)) || bytesRead != sizeof(MMPTE) || !pml4Entry.u.Hard.Valid)
					continue;

				MMPTE pdptEntry = { 0 };
				if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(
					(PVOID)((pml4Entry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pdpt_index),
					&pdptEntry,
					sizeof(MMPTE),
					&bytesRead)) || bytesRead != sizeof(MMPTE) || !pdptEntry.u.Hard.Valid)
					continue;

				MMPTE pdEntry = { 0 };
				if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(
					(PVOID)((pdptEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pd_index),
					&pdEntry,
					sizeof(MMPTE),
					&bytesRead)) || bytesRead != sizeof(MMPTE) || !pdEntry.u.Hard.Valid)
					continue;

				MMPTE ptEntry = { 0 };
				if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(
					(PVOID)((pdEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pt_index),
					&ptEntry,
					sizeof(MMPTE),
					&bytesRead)) || bytesRead != sizeof(MMPTE) || !ptEntry.u.Hard.Valid)
					continue;

				UINT64 physicalBase = TranslateLinearAddress(currentPhysical, target::BaseSectionAddress);
				if (!physicalBase)
					continue;

				char buffer[sizeof(_IMAGE_DOS_HEADER)] = { 0 };
				if (!NT_SUCCESS(ReadPhysicalMemoryWrapper(
					(PVOID)physicalBase,
					buffer,
					sizeof(_IMAGE_DOS_HEADER),
					&bytesRead)) || bytesRead != sizeof(_IMAGE_DOS_HEADER))
					continue;

				_IMAGE_DOS_HEADER* header = (_IMAGE_DOS_HEADER*)buffer;
				if (header->e_magic != IMAGE_DOS_SIGNATURE)
					continue;

				foundDTB = currentPhysical;
				break;
			}
			if (foundDTB)
				break;
		}
	}
	__finally
	{
		ExFreePool(physicalMemoryRanges);
	}

	return foundDTB;
}

namespace target
{
	inline PEPROCESS pTarget = nullptr;
	inline UINT64 BaseSectionAddress = 0;
	inline UINT64 DirectoryTableBase = 0;
	inline int RequestCount = 0;

	NTSTATUS ReadVirtualMemory(SystemRequest* Request)
	{
		if (!Request || !Request->Address || !Request->Buffer || !Request->Process || !Request->BufferSize)
			return STATUS_INVALID_PARAMETER;

		NTSTATUS status = STATUS_SUCCESS;
		//RequestCount++;
		//if (RequestCount >= 2000 || !DirectoryTableBase)
		//{
		//	if (!pTarget)
		//	{
		//		status = PsLookupProcessByProcessId((HANDLE)Request->Process, &pTarget);
		//		if (!NT_SUCCESS(status))
		//			return status;
		//	}
		//
		//	BaseSectionAddress = (UINT64)PsGetProcessSectionBaseAddress(pTarget);
		//	if (!BaseSectionAddress)
		//	{
		//		status = STATUS_INVALID_ADDRESS;
		//		return status;
		//	}
		//
		//	DirectoryTableBase = BruteForceDTB();
		//	if (!DirectoryTableBase)
		//	{
		//		status = STATUS_INVALID_ADDRESS;
		//		return status;
		//	}
		//	RequestCount = 0;
		//}

		//UINT64 Physical = TranslateLinearAddress(DirectoryTableBase, (UINT64)Request->Address);
		//if (!Physical)
		//{
		//	status = STATUS_INVALID_ADDRESS;
		//}
		//
		//SIZE_T alignedSize = min(PAGE_SIZE - (Physical & 0xFFF), Request->BufferSize);
		//if (alignedSize > PAGE_SIZE)
		//{
		//	status = STATUS_INVALID_LEVEL;
		//}

		Klog("Raw BaseSectionAddress: %llu", (UINT64)&BaseSectionAddress);
		Klog("Raw DirectoryTableBase: %llu", (UINT64)&DirectoryTableBase);
		Klog("Value BaseSectionAddress: %llu", (UINT64)BaseSectionAddress);
		Klog("Value DirectoryTableBase: %llu", (UINT64)DirectoryTableBase);

		// Test Klog with literals
		Klog("Test literal zero: %llu", (UINT64)0);
		Klog("Test literal one: %llu", (UINT64)1);

		SIZE_T sizeRead = 0;
		//status = ReadPhysicalMemoryWrapper((PVOID)Physical, Request->Buffer, alignedSize, &sizeRead);



		return status;
	}

}

NTSTATUS deviceiocontrol::IO_IRP_MJ_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(pIrp);
	SystemRequest* request = (SystemRequest*)pIrp->AssociatedIrp.SystemBuffer;
	NTSTATUS status = STATUS_SUCCESS;

	// Check if SystemBuffer is null or input buffer length is insufficient
	if (!pIrp->AssociatedIrp.SystemBuffer ||
		irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SystemRequest))
	{
		status = STATUS_INVALID_PARAMETER;
		goto complete;
	}

	if (irpStack->Parameters.DeviceIoControl.IoControlCode != DRIVER_CALL)
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		goto complete;
	}

	switch (request->CALL)
	{
	case SystemRequest::read:
		DbgPrint("[IO_IRP_MJ_DEVICE_CONTROL] Request - Address: 0x%p, Buffer: 0x%p, Size: %zu, PID: %d, Call: %d\n",
			request->Address, request->Buffer, request->BufferSize, request->Process, request->CALL);

		status = target::ReadVirtualMemory(request);

		pIrp->IoStatus.Information = NT_SUCCESS(status) ? request->BufferSize : 0;
		break;

	case SystemRequest::write:
		DbgPrint("[IO_IRP_MJ_DEVICE_CONTROL] Request - Address: 0x%p, Buffer: 0x%p, Size: %zu, PID: %d, Call: %d\n",
			request->Address, request->Buffer, request->BufferSize, request->Process, request->CALL);
		//status = target::WriteVirtualMemory(request);
		pIrp->IoStatus.Information = NT_SUCCESS(status) ? request->BufferSize : 0;
		break;

	default:
		status = STATUS_INVALID_PARAMETER;
		pIrp->IoStatus.Information = 0;
		break;
	}

complete:
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS deviceiocontrol::IO_IRP_MJ_CREATE(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceiocontrol::IO_IRP_MJ_CLOSE(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}