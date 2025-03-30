#ifndef KUTIL_H
#define KUTIL_H

#include "include.h"
#include "kport.h"

namespace kutil
{
    inline PKLDR_DATA_TABLE_ENTRY pGetModuleInfo(LPCWSTR ModuleName)
    {
        if (!ModuleName || IsListEmpty(PsLoadedModuleList))
            return nullptr;

        SIZE_T StringLength = wcslen(ModuleName) * sizeof(WCHAR) + sizeof(UNICODE_NULL);
        SIZE_T TotalSize = sizeof(UNICODE_STRING) + StringLength;

        PUNICODE_STRING ModuleNameUS = (PUNICODE_STRING)kport::ExAllocatePool2(
            POOL_FLAG_NON_PAGED,
            TotalSize,
            'vMQA'
        );

        if (!ModuleNameUS)
            return nullptr;

        ModuleNameUS->Buffer = (PWCH)((PUCHAR)ModuleNameUS + sizeof(UNICODE_STRING));
        RtlCopyMemory(ModuleNameUS->Buffer, ModuleName, StringLength);
        ModuleNameUS->Length = (USHORT)(StringLength - sizeof(UNICODE_NULL));
        ModuleNameUS->MaximumLength = (USHORT)StringLength;

        PKLDR_DATA_TABLE_ENTRY Result = nullptr;

        for (PLIST_ENTRY Entry = PsLoadedModuleList->Flink; Entry != PsLoadedModuleList; Entry = Entry->Flink)
        {
            PKLDR_DATA_TABLE_ENTRY Module = CONTAINING_RECORD(Entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            if (!kport::RtlCompareUnicodeString(&Module->BaseDllName, ModuleNameUS, FALSE))
            {
                Result = Module;
                break;
            }
        }

        kport::ExFreePoolWithTag(ModuleNameUS, 'vMQA');
        return Result;
    }
    inline PDEVICE_OBJECT pGetDeviceObject(LPCWSTR DosDevice)
    {
        if (!DosDevice)
            return nullptr;

        NTSTATUS Status;
        UNICODE_STRING ObjectName;
        PFILE_OBJECT File;
        PDEVICE_OBJECT Device;

        c_RtlInitUnicodeString(&ObjectName, DosDevice);

        Status = kport::IoGetDeviceObjectPointer(
            &ObjectName,
            FILE_READ_DATA,
            &File,
            &Device
        );

        if (!NT_SUCCESS(Status))
            return nullptr;

        kport::ObfDereferenceObject(File);

        return Device;
    }
    inline BOOLEAN IsKernelDebuggingEnable()
    {
        if (SharedUserData->KdDebuggerEnabled || KdDebuggerEnabled || !KdDebuggerNotPresent)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    
}

#endif KUTIL_H