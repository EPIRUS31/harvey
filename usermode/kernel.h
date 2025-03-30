#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>

inline BOOLEAN DEBUG = true;

inline void Ulog(const char* const _Format, ...) {
    if (!DEBUG)
        return;

    va_list args;
    va_start(args, _Format);
    printf(_Format, args);
    va_end(args);
}

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

const ULONG DRIVER_CALL = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS);

inline class _kernel
{
public:
    HANDLE kernelHandle = INVALID_HANDLE_VALUE;
    INT processHandle = 0;

    bool Attach(const wchar_t* ProcessName)
    {
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
            L"\\\\.\\harveygggg",
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

        return true;
    }

    void Detach()
    {
        if (kernelHandle != INVALID_HANDLE_VALUE)
        {
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

        DWORD bytesReturned;
        BOOL success = DeviceIoControl(
            kernelHandle,
            DRIVER_CALL,
            &Request,
            sizeof(Request),
            &Request,
            sizeof(Request),
            &bytesReturned,
            NULL
        );

        return success != FALSE;
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

        DWORD bytesReturned;
        BOOL success = DeviceIoControl(
            kernelHandle,
            DRIVER_CALL,
            &Request,
            sizeof(Request),
            &Request,
            sizeof(Request),
            &bytesReturned,
            NULL
        );

        return success != FALSE;
    }

    uintptr_t GetModuleBase(const wchar_t* ModuleName)
    {
        if (!processHandle )
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

} kernel;