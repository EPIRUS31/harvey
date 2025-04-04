#include "DeviceIoControl.h"

NTSTATUS ReadPhysicalMemoryWrapper(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesRead)
{
	if (!targetAddress || !buffer || size == 0 || !bytesRead)
		return STATUS_INVALID_PARAMETER;

	MM_COPY_ADDRESS copyAddress = { 0 };
	copyAddress.PhysicalAddress.QuadPart = reinterpret_cast<ULONGLONG>(targetAddress);

	return MmCopyMemory(buffer, copyAddress, size, MM_COPY_MEMORY_PHYSICAL, bytesRead);
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
	virtualBase.value = (void*)VirtualAddress; 
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

	if (pdEntry.u.Hard.LargePage)
	{
		return (pdEntry.u.Hard.PageFrameNumber << 21) | (VirtualAddress & 0x1FFFFF);
	}

	MMPTE ptEntry = { 0 };
	status = ReadPhysicalMemoryWrapper(
		(PVOID)((pdEntry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pt_index),
		&ptEntry,
		sizeof(MMPTE),
		&bytesRead
	);

	if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE) || !ptEntry.u.Hard.Valid)
		return 0;

	return (ptEntry.u.Hard.PageFrameNumber << 12) | (VirtualAddress & 0xFFF);
}

UINT64 BruteForceDTB()
{
	UINT64 baseAddress = target::BaseSectionAddress;
	if (!baseAddress)
	{
		return 0;
	}
	virt_addr_t virtualBase;
	virtualBase.value = (void*)baseAddress;

	PPHYSICAL_MEMORY_RANGE physicalRanges = MmGetPhysicalMemoryRanges();
	if (!physicalRanges)
	{
		return 0;
	}

	UINT64 foundDTB = 0;
	__try
	{
		for (int i = 0; ; i++)
		{
			PHYSICAL_MEMORY_RANGE currentRange = { 0 };
			RtlCopyMemory(&currentRange, &physicalRanges[i], sizeof(PHYSICAL_MEMORY_RANGE));
			if (!currentRange.BaseAddress.QuadPart || !currentRange.NumberOfBytes.QuadPart)
			{
				break;
			}

			UINT64 currentPhysical = currentRange.BaseAddress.QuadPart;

			for (int j = 0; j < (currentRange.NumberOfBytes.QuadPart / PAGE_SIZE); j++, currentPhysical += PAGE_SIZE)
			{
				MMPTE pml4Entry = { 0 };
				SIZE_T bytesRead = 0;
				UINT64 pml4Addr = currentPhysical + 8 * virtualBase.pml4_index;

				NTSTATUS status = ReadPhysicalMemoryWrapper(
					(PVOID)pml4Addr,
					&pml4Entry,
					sizeof(MMPTE),
					&bytesRead
				);
				if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE))
				{
					continue;
				}

				if (!pml4Entry.u.Hard.Valid)
					continue;

				MMPTE pdpte = { 0 };
				status = ReadPhysicalMemoryWrapper(
					(PVOID)((pml4Entry.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pdpt_index),
					&pdpte,
					sizeof(MMPTE),
					&bytesRead
				);
				if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE))
				{
					continue;
				}

				if (!pdpte.u.Hard.Valid)
					continue;

				MMPTE pde = { 0 };
				status = ReadPhysicalMemoryWrapper(
					(PVOID)((pdpte.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pd_index),
					&pde,
					sizeof(MMPTE),
					&bytesRead
				);
				if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE))
				{
					continue;
				}

				if (!pde.u.Hard.Valid)
					continue;

				MMPTE pte = { 0 };
				status = ReadPhysicalMemoryWrapper(
					(PVOID)((pde.u.Hard.PageFrameNumber << 12) + 8 * virtualBase.pt_index),
					&pte,
					sizeof(MMPTE),
					&bytesRead
				);
				if (!NT_SUCCESS(status) || bytesRead != sizeof(MMPTE))
				{
					continue;
				}

				if (!pte.u.Hard.Valid)
					continue;

				UINT64 physicalBase = TranslateLinearAddress(currentPhysical, baseAddress);
				if (!physicalBase)
				{
					continue;
				}

				char buffer[sizeof(IMAGE_DOS_HEADER)];
				status = ReadPhysicalMemoryWrapper(
					(PVOID)physicalBase,
					buffer,
					sizeof(IMAGE_DOS_HEADER),
					&bytesRead
				);
				if (!NT_SUCCESS(status) || bytesRead != sizeof(_IMAGE_DOS_HEADER))
				{
					continue;
				}

				PIMAGE_DOS_HEADER header = reinterpret_cast<PIMAGE_DOS_HEADER>(buffer);
				if (header->e_magic != IMAGE_DOS_SIGNATURE)
					continue;

				foundDTB = currentPhysical;
				break;
			}
			if (foundDTB)
				break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		foundDTB = 0;
	}

	ExFreePool(physicalRanges);
	return foundDTB;
}

typedef struct _CACHE_DTB_WORK_CONTEXT {
	PDEVICE_OBJECT DeviceObject;    
	SystemRequest* Request;        
	PIO_WORKITEM WorkItem;      
	KEVENT CompletionEvent;         
	NTSTATUS Status;               
} CACHE_DTB_WORK_CONTEXT, * PCACHE_DTB_WORK_CONTEXT;

VOID CacheDtbWorkerRoutine(
	PDEVICE_OBJECT DeviceObject,
	PVOID Context
)
{
	PCACHE_DTB_WORK_CONTEXT workContext = (PCACHE_DTB_WORK_CONTEXT)Context;

	NTSTATUS status = STATUS_SUCCESS;

	if (!workContext->Request || !workContext->Request->Process) {
		status = STATUS_INVALID_PARAMETER;
		goto cleanup;
	}

	if (target::pTarget) {
		ObDereferenceObject(target::pTarget);
		target::pTarget = nullptr;
	}

	status = PsLookupProcessByProcessId((HANDLE)workContext->Request->Process, &target::pTarget);
	if (!NT_SUCCESS(status)) {
		goto cleanup;
	}

	target::BaseSectionAddress = (UINT64)PsGetProcessSectionBaseAddress(target::pTarget);
	if (!target::BaseSectionAddress) {
		ObDereferenceObject(target::pTarget);
		target::pTarget = nullptr;
		status = STATUS_INVALID_ADDRESS;
		goto cleanup;
	}

	target::DirectoryTableBase = BruteForceDTB();
	if (!target::DirectoryTableBase) {
		ObDereferenceObject(target::pTarget);
		target::pTarget = nullptr;
		status = STATUS_INVALID_ADDRESS;
		goto cleanup;
	}

cleanup:
	workContext->Status = status;

	KeSetEvent(&workContext->CompletionEvent, IO_NO_INCREMENT, FALSE);
	IoFreeWorkItem(workContext->WorkItem);
	ExFreePool(workContext);
}

NTSTATUS CacheDtb(SystemRequest* Request, PDEVICE_OBJECT DeviceObject)
{
	if (!Request || !Request->Process) {
		return STATUS_INVALID_PARAMETER;
	}

	PCACHE_DTB_WORK_CONTEXT workContext = (PCACHE_DTB_WORK_CONTEXT)ExAllocatePoolWithTag(
		NonPagedPool,
		sizeof(CACHE_DTB_WORK_CONTEXT),
		'DTBC'
	);
	if (!workContext) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	workContext->DeviceObject = DeviceObject;
	workContext->Request = Request;
	workContext->WorkItem = IoAllocateWorkItem(DeviceObject);
	workContext->Status = STATUS_PENDING;
	KeInitializeEvent(&workContext->CompletionEvent, NotificationEvent, FALSE);

	if (!workContext->WorkItem) {
		ExFreePool(workContext);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	IoQueueWorkItem(
		workContext->WorkItem,
		CacheDtbWorkerRoutine,
		DelayedWorkQueue,
		workContext
	);

	return STATUS_PENDING;
}

namespace target
{
	inline PEPROCESS pTarget = nullptr;
	inline UINT64 BaseSectionAddress = 0;
	inline UINT64 DirectoryTableBase = 0;

	NTSTATUS ReadVirtualMemory(SystemRequest* Request)
	{
		if (!Request || !Request->Address || !Request->Buffer || !Request->Process || !Request->BufferSize)
			return STATUS_INVALID_PARAMETER;

		NTSTATUS status = STATUS_SUCCESS;

		if (!DirectoryTableBase)
		{
			return STATUS_INVALID_ADDRESS;
		}

		UINT64 Physical = TranslateLinearAddress(DirectoryTableBase, (UINT64)Request->Address);
		if (!Physical)
		{
			status = STATUS_INVALID_ADDRESS;
			return status;
		}

		SIZE_T alignedSize = min(PAGE_SIZE - (Physical & 0xFFF), Request->BufferSize);
		if (alignedSize > PAGE_SIZE)
		{
			status = STATUS_INVALID_LEVEL;
			return status;
		}

		SIZE_T sizeRead = 0;

		status = ReadPhysicalMemoryWrapper((PVOID)Physical, Request->Buffer, alignedSize, &sizeRead);

		return status;
	}
	NTSTATUS WriteVirtualMemory(SystemRequest* Request)
	{
		if (!Request || !Request->Address || !Request->Buffer || !Request->Process || !Request->BufferSize)
			return STATUS_INVALID_PARAMETER;

		NTSTATUS status = STATUS_SUCCESS;

		if (!DirectoryTableBase)
		{
			return STATUS_INVALID_ADDRESS;
		}

		UINT64 Physical = TranslateLinearAddress(DirectoryTableBase, (UINT64)Request->Address);
		if (!Physical)
		{
			status = STATUS_INVALID_ADDRESS;
			return status;
		}

		SIZE_T alignedSize = min(PAGE_SIZE - (Physical & 0xFFF), Request->BufferSize);
		if (alignedSize > PAGE_SIZE)
		{
			status = STATUS_INVALID_LEVEL;
			return status;
		}

		SIZE_T sizeRead = 0;

		status = WritePhysicalMemoryWrapper((PVOID)Physical, Request->Buffer, alignedSize, &sizeRead);

		return status;
	}
}

NTSTATUS deviceiocontrol::IO_IRP_MJ_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(pIrp);
	SystemRequest* request = (SystemRequest*)pIrp->AssociatedIrp.SystemBuffer;
	NTSTATUS status = STATUS_SUCCESS;

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
		status = target::ReadVirtualMemory(request);
		pIrp->IoStatus.Information = NT_SUCCESS(status) ? request->BufferSize : 0;
		break;

	case SystemRequest::write:
		status = target::WriteVirtualMemory(request);
		pIrp->IoStatus.Information = NT_SUCCESS(status) ? request->BufferSize : 0;
		break;
	case SystemRequest::cache:
		status = CacheDtb(request, pDeviceObject);
		Klog(X("target::DirectoryTableBase = %p"), target::DirectoryTableBase);
		Klog(X("target::Base = %p"), target::BaseSectionAddress);
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