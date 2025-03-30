#ifndef KPORT_H
#define KPORT_H

#include "include.h"

namespace kportstd
{
    template <class _Ty>
    struct remove_reference {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_reference<_Ty&> {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_reference<_Ty&&> {
        using type = _Ty;
    };

    template <class _Ty>
    using remove_reference_t = typename remove_reference<_Ty>::type;

    template <class _Ty>
    struct remove_const {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_const<const _Ty> {
        using type = _Ty;
    };

    template <class _Ty>
    using remove_const_t = typename remove_const<_Ty>::type;
}
namespace kport_crypt
{
    template<class _Ty>
    using clean_type = typename kportstd::remove_const_t<kportstd::remove_reference_t<_Ty>>;

    template <int _size, char _key1, char _key2, typename T>
    class kport_strer
    {
    public:
        __forceinline constexpr kport_strer(T* data)
        {
            crypt(data);
        }

        __forceinline T* get()
        {
            return _storage;
        }

        __forceinline int size()
        {
            return _size;
        }

        __forceinline char key()
        {
            return _key1;
        }

        __forceinline T* encrypt()
        {
            if (!isEncrypted())
                crypt(_storage);
            return _storage;
        }

        __forceinline T* decrypt()
        {
            if (isEncrypted())
                crypt(_storage);
            return _storage;
        }

        __forceinline bool isEncrypted()
        {
            return _storage[_size - 1] != 0;
        }

        __forceinline void clear()
        {
            for (int i = 0; i < _size; i++)
            {
                _storage[i] = 0;
            }
        }

        __forceinline operator T* ()
        {
            decrypt();
            return _storage;
        }

    private:
        __forceinline constexpr void crypt(T* data)
        {
            for (int i = 0; i < _size; i++)
            {
                _storage[i] = data[i] ^ (_key1 + i % (1 + _key2));
            }
        }

        T _storage[_size]{};
    };
}

#define kport_str(str) kport_key(str, __TIME__[4], __TIME__[7])
#define kport_key(str, key1, key2) []() { \
    constexpr static auto crypted = kport_crypt::kport_strer \
        <sizeof(str) / sizeof(str[0]), key1, key2, kport_crypt::clean_type<decltype(str[0])>>((kport_crypt::clean_type<decltype(str[0])>*)str); \
    return crypted; }()

inline void __stdcall c_RtlInitUnicodeString(PUNICODE_STRING DestinationString, PCWSTR SourceString)
{
    __int64 v2;
    unsigned __int64 v3;

    *(DWORD64*)&DestinationString->Length = 0LL;
    DestinationString->Buffer = (wchar_t*)SourceString;
    if (SourceString)
    {
        v2 = -1LL;
        do
            ++v2;
        while (SourceString[v2]);
        v3 = 2 * v2;
        if (v3 >= 0xFFFE)
            v3 = 0xFFFE;
        DestinationString->Length = (USHORT)v3;
        DestinationString->MaximumLength = (USHORT)(v3 + 2);
    }
}
inline PIO_STACK_LOCATION c_IoGetCurrentIrpStackLocation(_In_ PIRP Irp)
{
    NT_ASSERT(Irp->CurrentLocation <= Irp->StackCount + 1);
    return Irp->Tail.Overlay.CurrentStackLocation;
}

typedef NTSTATUS(*IofCallDriver_t)(PDEVICE_OBJECT DeviceObject, PIRP Irp);
typedef VOID(*IoDeleteDevice_t)(PDEVICE_OBJECT DeviceObject);
typedef NTSTATUS(*IoDeleteSymbolicLink_t)(PUNICODE_STRING SymbolicLinkName);
typedef NTSTATUS(*IoCreateDriver_t)(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
typedef NTSTATUS(*IoCreateSymbolicLink_t)(PUNICODE_STRING SymbolicLinkName, PUNICODE_STRING DeviceName);
typedef NTSTATUS(*IoCreateDevice_t)(PDRIVER_OBJECT DriverObject, ULONG DeviceExtensionSize, PUNICODE_STRING DeviceName, DEVICE_TYPE DeviceType, ULONG DeviceCharacteristics, BOOLEAN Exclusive, PDEVICE_OBJECT* DeviceObject);
typedef PHYSICAL_MEMORY_RANGE* (*MmGetPhysicalMemoryRanges_t)();
typedef NTSTATUS(*MmCopyMemory_t)(PVOID Destination, MM_COPY_ADDRESS Source, SIZE_T Length, ULONG Flags, SIZE_T* BytesWritten);
typedef PVOID(*MmMapIoSpaceEx_t)(PHYSICAL_ADDRESS PhysicalAddress, SIZE_T NumberOfBytes, ULONG Protect);
typedef VOID(*MmUnmapIoSpace_t)(PVOID BaseAddress, SIZE_T NumberOfBytes);
typedef KIRQL(*KeAcquireSpinLockRaiseToDpc_t)(_Inout_ PKSPIN_LOCK SpinLock);
typedef VOID(*KeAcquireSpinLockAtDpcLevel_t)(_Inout_ PKSPIN_LOCK SpinLock);
typedef VOID(*KeReleaseSpinLockFromDpcLevel_t)(_Inout_ PKSPIN_LOCK SpinLock);
typedef KIRQL(*KeGetCurrentIrql_t)(VOID);
typedef VOID(*KeRaiseIrql_t)(KIRQL NewIrql, PKIRQL OldIrql);
typedef VOID(*KeLowerIrql_t)(KIRQL NewIrql);
typedef BOOLEAN(*MmIsAddressValid_t)(PVOID VirtualAddress);
typedef PVOID(*ExAllocatePool2_t)(POOL_FLAGS PoolType, SIZE_T NumberOfBytes, ULONG Tag);
typedef VOID(*ExFreePoolWithTag_t)(PVOID P, ULONG Tag);
typedef VOID(*ExFreePool_t)(PVOID P);
typedef LONG(*RtlCompareUnicodeString_t)(PUNICODE_STRING String1, PUNICODE_STRING String2, BOOLEAN CaseInSensitive);
typedef VOID(*KeInitializeSpinLock_t)(PKSPIN_LOCK SpinLock);
typedef VOID(*ObDereferenceObjectDeferDelete_t)(PVOID Object);
typedef BOOLEAN(*ObReferenceObjectSafe_t)(PVOID Object);
typedef VOID(*KeReleaseSpinLock_t)(PKSPIN_LOCK SpinLock, KIRQL NewIrql);
typedef NTSTATUS(*IoGetDeviceObjectPointer_t)(PUNICODE_STRING ObjectName, ACCESS_MASK DesiredAccess, PFILE_OBJECT* FileObject, PDEVICE_OBJECT* DeviceObject);
typedef KIRQL(*KfRaiseIrql_t)(KIRQL NewIrql);
typedef LONG_PTR(*ObfDereferenceObject_t)(_In_ PVOID Object);

namespace kport
{
    static IofCallDriver_t IofCallDriver = nullptr;
    static IoDeleteDevice_t IoDeleteDevice = nullptr;
    static IoDeleteSymbolicLink_t IoDeleteSymbolicLink = nullptr;
    static IoCreateDriver_t IoCreateDriver = nullptr;
    static IoCreateSymbolicLink_t IoCreateSymbolicLink = nullptr;
    static IoCreateDevice_t IoCreateDevice = nullptr;
    static MmGetPhysicalMemoryRanges_t MmGetPhysicalMemoryRanges = nullptr;
    static MmCopyMemory_t MmCopyMemory = nullptr;
    static MmMapIoSpaceEx_t MmMapIoSpaceEx = nullptr;
    static MmUnmapIoSpace_t MmUnmapIoSpace = nullptr;
    static MmIsAddressValid_t MmIsAddressValid = nullptr;
    static ExAllocatePool2_t ExAllocatePool2 = nullptr;
    static ExFreePoolWithTag_t ExFreePoolWithTag = nullptr;
    static ExFreePool_t ExFreePool = nullptr;
    static RtlCompareUnicodeString_t RtlCompareUnicodeString = nullptr;
    static ObDereferenceObjectDeferDelete_t ObDereferenceObjectDeferDelete = nullptr;
    static ObReferenceObjectSafe_t ObReferenceObjectSafe = nullptr;
    static KeReleaseSpinLock_t KeReleaseSpinLock = nullptr;
    static IoGetDeviceObjectPointer_t IoGetDeviceObjectPointer = nullptr;
    static ObfDereferenceObject_t ObfDereferenceObject = nullptr;

    inline void* resolveKernel(LPWSTR FunctionName)
    {
        struct {
            ULONG_PTR padding1;
            USHORT length;
            USHORT maxLength;
            PWSTR buffer;
            ULONG_PTR padding2;
        } customStack;

        UNICODE_STRING* _x = (UNICODE_STRING*)((char*)&customStack + sizeof(ULONG_PTR));

        customStack.padding1 = (ULONG_PTR)FunctionName;
        customStack.padding2 = (ULONG_PTR)_x;

        struct {
            ULONG_PTR shadowSpace[4];
            UNICODE_STRING* param1;
            PCWSTR param2;
        } rtlCallFrame;
        rtlCallFrame.shadowSpace[0] = 0;
        rtlCallFrame.shadowSpace[1] = 0;
        rtlCallFrame.shadowSpace[2] = 0;
        rtlCallFrame.shadowSpace[3] = 0;
        rtlCallFrame.param1 = _x;
        rtlCallFrame.param2 = FunctionName;

        c_RtlInitUnicodeString(rtlCallFrame.param1, rtlCallFrame.param2);

        void* mmGetAddr = MmGetSystemRoutineAddress;

        if (_x->Buffer) {
            struct {
                ULONG_PTR shadowSpace[4];
                UNICODE_STRING* param1;
            } mmCallFrame;
            mmCallFrame.shadowSpace[0] = 0;
            mmCallFrame.shadowSpace[1] = 0;
            mmCallFrame.shadowSpace[2] = 0;
            mmCallFrame.shadowSpace[3] = 0;
            mmCallFrame.param1 = _x;

            void* result = ((void* (*)(UNICODE_STRING*))(mmGetAddr))(mmCallFrame.param1);
            return result;
        }
        return nullptr;
    }

    inline void Kernel()
    {
        LPWSTR name;

        name = kport_str(L"IofCallDriver");
        IofCallDriver = reinterpret_cast<IofCallDriver_t>(resolveKernel(name));

        name = kport_str(L"IoDeleteDevice");
        IoDeleteDevice = reinterpret_cast<IoDeleteDevice_t>(resolveKernel(name));

        name = kport_str(L"IoDeleteSymbolicLink");
        IoDeleteSymbolicLink = reinterpret_cast<IoDeleteSymbolicLink_t>(resolveKernel(name));

        name = kport_str(L"IoCreateDriver");
        IoCreateDriver = reinterpret_cast<IoCreateDriver_t>(resolveKernel(name));

        name = kport_str(L"IoCreateSymbolicLink");
        IoCreateSymbolicLink = reinterpret_cast<IoCreateSymbolicLink_t>(resolveKernel(name));

        name = kport_str(L"IoCreateDevice");
        IoCreateDevice = reinterpret_cast<IoCreateDevice_t>(resolveKernel(name));

        name = kport_str(L"MmGetPhysicalMemoryRanges");
        MmGetPhysicalMemoryRanges = reinterpret_cast<MmGetPhysicalMemoryRanges_t>(resolveKernel(name));

        name = kport_str(L"MmCopyMemory");
        MmCopyMemory = reinterpret_cast<MmCopyMemory_t>(resolveKernel(name));

        name = kport_str(L"MmMapIoSpaceEx");
        MmMapIoSpaceEx = reinterpret_cast<MmMapIoSpaceEx_t>(resolveKernel(name));

        name = kport_str(L"MmUnmapIoSpace");
        MmUnmapIoSpace = reinterpret_cast<MmUnmapIoSpace_t>(resolveKernel(name));

        name = kport_str(L"MmIsAddressValid");
        MmIsAddressValid = reinterpret_cast<MmIsAddressValid_t>(resolveKernel(name));

        name = kport_str(L"ExAllocatePool2");
        ExAllocatePool2 = reinterpret_cast<ExAllocatePool2_t>(resolveKernel(name));

        name = kport_str(L"ExFreePoolWithTag");
        ExFreePoolWithTag = reinterpret_cast<ExFreePoolWithTag_t>(resolveKernel(name));

        name = kport_str(L"ExFreePool");
        ExFreePool = reinterpret_cast<ExFreePool_t>(resolveKernel(name));

        name = kport_str(L"RtlCompareUnicodeString");
        RtlCompareUnicodeString = reinterpret_cast<RtlCompareUnicodeString_t>(resolveKernel(name));

        name = kport_str(L"ObDereferenceObjectDeferDelete");
        ObDereferenceObjectDeferDelete = reinterpret_cast<ObDereferenceObjectDeferDelete_t>(resolveKernel(name));

        name = kport_str(L"ObReferenceObjectSafe");
        ObReferenceObjectSafe = reinterpret_cast<ObReferenceObjectSafe_t>(resolveKernel(name));

        name = kport_str(L"KeReleaseSpinLock");
        KeReleaseSpinLock = reinterpret_cast<KeReleaseSpinLock_t>(resolveKernel(name));

        name = kport_str(L"IoGetDeviceObjectPointer");
        IoGetDeviceObjectPointer = reinterpret_cast<IoGetDeviceObjectPointer_t>(resolveKernel(name));

        name = kport_str(L"ObfDereferenceObject");
        ObfDereferenceObject = reinterpret_cast<ObfDereferenceObject_t>(resolveKernel(name));
    }

}


#endif // !KPORT_H