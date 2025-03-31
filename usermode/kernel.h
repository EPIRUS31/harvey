#ifndef KERNEL_H
#define KERNEL_H

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <thread>
#include "ntloadup.h"

typedef NTSTATUS(
NTAPI*
NtDeviceIoControlFile_t)(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
);

inline BOOLEAN DEBUG = false;

inline void Ulog(const char* const _Format, ...) {
    if (!DEBUG)
        return;

    va_list args;
    va_start(args, _Format);
    vprintf(_Format, args);
    va_end(args);
}

struct SystemRequest
{
    PVOID Address;
    PVOID Buffer;
    SIZE_T BufferSize;
    INT Process;
    enum _CALL
    {
        read,
        write,
        cache
    } CALL;
};

const ULONG DRIVER_CALL = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS);

inline class _kernel
{
public:
    HANDLE kernelHandle = INVALID_HANDLE_VALUE;
    INT processHandle = 0;
    std::thread caching;
    bool isCaching = false;
    NtDeviceIoControlFile_t NtDeviceIoControlFileImport;

    bool CacheProcessDirectoryTableBase()
    {
        if (kernelHandle == INVALID_HANDLE_VALUE)
            return false;

        SystemRequest Request{};
        Request.Process = processHandle;
        Request.CALL = SystemRequest::_CALL::cache;

        IO_STATUS_BLOCK ioStatus;
        NTSTATUS status = NtDeviceIoControlFileImport(
            kernelHandle,
            NULL,
            NULL,
            NULL,
            &ioStatus,
            DRIVER_CALL,
            &Request,
            sizeof(Request),
            &Request,
            sizeof(Request)
        );

        return NT_SUCCESS(status);
    }

    void CacheThread()
    {
        while (isCaching)
        {
            CacheProcessDirectoryTableBase();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    bool Attach(const wchar_t* ProcessName)
    {
        HMODULE NTDLL = GetModuleHandleA("ntdll.dll");
        if (!NTDLL) return false;

        NtDeviceIoControlFileImport = (NtDeviceIoControlFile_t)GetProcAddress(NTDLL, "NtDeviceIoControlFile");
        if (!NtDeviceIoControlFileImport)
            return false;

        if (!driver::isloaded())
        {
            driver::load(driver::rawData, sizeof(driver::rawData));
        }

        HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (SnapShot == INVALID_HANDLE_VALUE)
            return false;

        PROCESSENTRY32W Entry{};
        Entry.dwSize = sizeof(PROCESSENTRY32W);

        BOOL success = Process32FirstW(SnapShot, &Entry);
        while (success)
        {
            if (_wcsicmp(Entry.szExeFile, ProcessName) == 0)
            {
                processHandle = Entry.th32ProcessID;
                break;
            }
            success = Process32NextW(SnapShot, &Entry);
        }
        CloseHandle(SnapShot);

        if (processHandle == 0)
        {
            Ulog("failed to find process\n");
            return false;
        }

        kernelHandle = CreateFileW(
            L"\\\\.\\sigmadriver",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (kernelHandle == INVALID_HANDLE_VALUE)
        {
            Ulog("failed to get driver handle\n");
            return false;
        }

        
        isCaching = true;
        caching = std::thread(&_kernel::CacheThread, this);
        caching.detach();

        return true;
    }

    void Detach()
    {
        if (kernelHandle != INVALID_HANDLE_VALUE)
        {
            isCaching = false;
            CloseHandle(kernelHandle);
            kernelHandle = INVALID_HANDLE_VALUE;
        }
        processHandle = 0;
    }

    bool ReadVirtualMemory(uintptr_t Address, void* Buffer, SIZE_T Size)
    {
        if (kernelHandle == INVALID_HANDLE_VALUE)
            return false;

        SystemRequest Request{};
        Request.Address = (PVOID)Address;
        Request.Buffer = Buffer;
        Request.BufferSize = Size;
        Request.Process = processHandle;
        Request.CALL = SystemRequest::_CALL::read;

        IO_STATUS_BLOCK ioStatus;
        NTSTATUS status = NtDeviceIoControlFileImport(
            kernelHandle,
            NULL,
            NULL,
            NULL,
            &ioStatus,
            DRIVER_CALL,
            &Request,
            sizeof(Request),
            &Request,
            sizeof(Request)
        );

        return NT_SUCCESS(status);
    }

    bool WriteVirtualMemory(uintptr_t Address, void* Buffer, SIZE_T Size)
    {
        if (kernelHandle == INVALID_HANDLE_VALUE)
            return false;

        SystemRequest Request{};
        Request.Address = (PVOID)Address;
        Request.Buffer = Buffer;
        Request.BufferSize = Size;
        Request.Process = processHandle;
        Request.CALL = SystemRequest::_CALL::write;

        IO_STATUS_BLOCK ioStatus;
        NTSTATUS status = NtDeviceIoControlFileImport(
            kernelHandle,
            NULL,
            NULL,
            NULL,
            &ioStatus,
            DRIVER_CALL,
            &Request,
            sizeof(Request),
            &Request,
            sizeof(Request)
        );

        return NT_SUCCESS(status);
    }

    uintptr_t GetModuleBase(const wchar_t* ModuleName)
    {
        if (!processHandle)
            return 0;

        HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processHandle);
        if (SnapShot == INVALID_HANDLE_VALUE)
            return 0;

        MODULEENTRY32W ModuleEntry{};
        ModuleEntry.dwSize = sizeof(MODULEENTRY32W);

        BOOL success = Module32FirstW(SnapShot, &ModuleEntry);
        while (success)
        {
            if (_wcsicmp(ModuleEntry.szModule, ModuleName) == 0)
            {
                CloseHandle(SnapShot);
                return (uintptr_t)ModuleEntry.modBaseAddr;
            }
            success = Module32NextW(SnapShot, &ModuleEntry);
        }

        CloseHandle(SnapShot);
        return 0;
    }


    template<typename T> T read(uintptr_t Address)
    {
        T Buffer{};
        this->ReadVirtualMemory(Address, &Buffer, sizeof(T));
        return Buffer;
    }

    template<typename T> void write(uintptr_t Address, const T& Buffer)
    {
        this->WriteVirtualMemory(Address, &Buffer, sizeof(T));
    }

} kernel;



#endif // !KERNEL_H