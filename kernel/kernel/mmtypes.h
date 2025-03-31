#ifndef MMTYPES
#define MMTYPES

typedef struct _KLDR_DATA_TABLE_ENTRY
{
    struct _LIST_ENTRY InLoadOrderLinks;
    VOID* ExceptionTable;
    ULONG ExceptionTableSize;
    VOID* GpValue;
    struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;
    VOID* DllBase;
    VOID* EntryPoint;
    ULONG SizeOfImage;
    struct _UNICODE_STRING FullDllName;
    struct _UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    union
    {
        USHORT SignatureLevel : 4;
        USHORT SignatureType : 3;
        USHORT Unused : 9;
        USHORT EntireField;
    } u1;
    VOID* SectionPointer;
    ULONG CheckSum;
    ULONG CoverageSectionSize;
    VOID* CoverageSection;
    VOID* LoadedImports;
    VOID* Spare;
    ULONG SizeOfImageNotRounded;
    ULONG TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

#define IMAGE_DOS_SIGNATURE                 0x5A4D

typedef struct _IMAGE_DOS_HEADER
{
    WORD e_magic;
    WORD e_cblp;
    WORD e_cp;
    WORD e_crlc;
    WORD e_cparhdr;
    WORD e_minalloc;
    WORD e_maxalloc;
    WORD e_ss;
    WORD e_sp;
    WORD e_csum;
    WORD e_ip;
    WORD e_cs;
    WORD e_lfarlc;
    WORD e_ovno;
    WORD e_res[4];
    WORD e_oemid;
    WORD e_oeminfo;
    WORD e_res2[10];
    LONG e_lfanew;
} IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER;

typedef union _virt_addr_t
{
    void* value;
    struct
    {
        uintptr_t offset : 12;
        uintptr_t pt_index : 9;
        uintptr_t pd_index : 9;
        uintptr_t pdpt_index : 9;
        uintptr_t pml4_index : 9;
        uintptr_t reserved : 16;
    };
} virt_addr_t, * pvirt_addr_t;

typedef struct _MI_ACTIVE_PFN
{
    union
    {
        struct
        {
            struct
            {
                unsigned __int64 Tradable : 1;
                unsigned __int64 NonPagedBuddy : 43;
            };
        } Leaf;
        struct
        {
            struct
            {
                unsigned __int64 Tradable : 1;
                unsigned __int64 WsleAge : 3;
                unsigned __int64 OldestWsleLeafEntries : 10;
                unsigned __int64 OldestWsleLeafAge : 3;
                unsigned __int64 NonPagedBuddy : 43;
            };
        } PageTable;
        unsigned __int64 EntireActiveField;
    };
} MI_ACTIVE_PFN, * PMI_ACTIVE_PFN;
typedef struct _MMPTE_HARDWARE
{
    struct
    {
        unsigned __int64 Valid : 1;
        unsigned __int64 Dirty1 : 1;
        unsigned __int64 Owner : 1;
        unsigned __int64 WriteThrough : 1;
        unsigned __int64 CacheDisable : 1;
        unsigned __int64 Accessed : 1;
        unsigned __int64 Dirty : 1;
        unsigned __int64 LargePage : 1;
        unsigned __int64 Global : 1;
        unsigned __int64 CopyOnWrite : 1;
        unsigned __int64 Unused : 1;
        unsigned __int64 Write : 1;
        unsigned __int64 PageFrameNumber : 40;
        unsigned __int64 ReservedForSoftware : 4;
        unsigned __int64 WsleAge : 4;
        unsigned __int64 WsleProtection : 3;
        unsigned __int64 NoExecute : 1;
    };
} MMPTE_HARDWARE, * PMMPTE_HARDWARE;
typedef struct _MMPTE_PROTOTYPE
{
    struct
    {
        unsigned __int64 Valid : 1;
        unsigned __int64 DemandFillProto : 1;
        unsigned __int64 HiberVerifyConverted : 1;
        unsigned __int64 ReadOnly : 1;
        unsigned __int64 SwizzleBit : 1;
        unsigned __int64 Protection : 5;
        unsigned __int64 Prototype : 1;
        unsigned __int64 Combined : 1;
        unsigned __int64 Unused1 : 4;
        __int64 ProtoAddress : 48;
    };
} MMPTE_PROTOTYPE, * PMMPTE_PROTOTYPE;
typedef struct _MMPTE_SOFTWARE
{
    struct
    {
        unsigned __int64 Valid : 1;
        unsigned __int64 PageFileReserved : 1;
        unsigned __int64 PageFileAllocated : 1;
        unsigned __int64 ColdPage : 1;
        unsigned __int64 SwizzleBit : 1;
        unsigned __int64 Protection : 5;
        unsigned __int64 Prototype : 1;
        unsigned __int64 Transition : 1;
        unsigned __int64 PageFileLow : 4;
        unsigned __int64 UsedPageTableEntries : 10;
        unsigned __int64 ShadowStack : 1;
        unsigned __int64 Unused : 5;
        unsigned __int64 PageFileHigh : 32;
    };
} MMPTE_SOFTWARE, * PMMPTE_SOFTWARE;
typedef struct _MMPTE_TIMESTAMP
{
    struct
    {
        unsigned __int64 MustBeZero : 1;
        unsigned __int64 Unused : 3;
        unsigned __int64 SwizzleBit : 1;
        unsigned __int64 Protection : 5;
        unsigned __int64 Prototype : 1;
        unsigned __int64 Transition : 1;
        unsigned __int64 PageFileLow : 4;
        unsigned __int64 Reserved : 16;
        unsigned __int64 GlobalTimeStamp : 32;
    };
} MMPTE_TIMESTAMP, * PMMPTE_TIMESTAMP;
typedef struct _MMPTE_TRANSITION
{
    struct
    {
        unsigned __int64 Valid : 1;
        unsigned __int64 Write : 1;
        unsigned __int64 Spare : 1;
        unsigned __int64 IoTracker : 1;
        unsigned __int64 SwizzleBit : 1;
        unsigned __int64 Protection : 5;
        unsigned __int64 Prototype : 1;
        unsigned __int64 Transition : 1;
        unsigned __int64 PageFrameNumber : 40;
        unsigned __int64 Unused : 12;
    };
} MMPTE_TRANSITION, * PMMPTE_TRANSITION;
typedef struct _MMPTE_SUBSECTION
{
    struct
    {
        unsigned __int64 Valid : 1;
        unsigned __int64 Unused0 : 3;
        unsigned __int64 SwizzleBit : 1;
        unsigned __int64 Protection : 5;
        unsigned __int64 Prototype : 1;
        unsigned __int64 ColdPage : 1;
        unsigned __int64 Unused1 : 3;
        unsigned __int64 ExecutePrivilege : 1;
        __int64 SubsectionAddress : 48;
    };
} MMPTE_SUBSECTION, * PMMPTE_SUBSECTION;
typedef struct _MMPTE_LIST
{
    struct
    {
        unsigned __int64 Valid : 1;
        unsigned __int64 OneEntry : 1;
        unsigned __int64 filler0 : 2;
        unsigned __int64 SwizzleBit : 1;
        unsigned __int64 Protection : 5;
        unsigned __int64 Prototype : 1;
        unsigned __int64 Transition : 1;
        unsigned __int64 filler1 : 16;
        unsigned __int64 NextEntry : 36;
    };
} MMPTE_LIST, * PMMPTE_LIST;
typedef struct _MMPTE
{
    union
    {
        union
        {
            unsigned __int64 Long;
            volatile unsigned __int64 VolatileLong;
            struct _MMPTE_HARDWARE Hard;
            struct _MMPTE_PROTOTYPE Proto;
            struct _MMPTE_SOFTWARE Soft;
            struct _MMPTE_TIMESTAMP TimeStamp;
            struct _MMPTE_TRANSITION Trans;
            struct _MMPTE_SUBSECTION Subsect;
            struct _MMPTE_LIST List;
        };
    } u;
} MMPTE, * PMMPTE;
typedef struct _MIPFNBLINK
{
    union
    {
        struct
        {
            unsigned __int64 Blink : 40;
            unsigned __int64 NodeBlinkLow : 19;
            unsigned __int64 TbFlushStamp : 3;
            unsigned __int64 PageBlinkDeleteBit : 1;
            unsigned __int64 PageBlinkLockBit : 1;
        };
        struct
        {
            unsigned __int64 ShareCount : 62;
            unsigned __int64 PageShareCountDeleteBit : 1;
            unsigned __int64 PageShareCountLockBit : 1;
        };
        unsigned __int64 EntireField;
        volatile __int64 Lock;
        struct
        {
            unsigned __int64 LockNotUsed : 62;
            unsigned __int64 DeleteBit : 1;
            unsigned __int64 LockBit : 1;
        };
    };
} MIPFNBLINK, * PMIPFNBLINK;
typedef struct _MMPFNENTRY1
{
    struct
    {
        unsigned char PageLocation : 3;
        unsigned char WriteInProgress : 1;
        unsigned char Modified : 1;
        unsigned char ReadInProgress : 1;
        unsigned char CacheAttribute : 2;
    };
} MMPFNENTRY1, * PMMPFNENTRY1;
typedef struct _MMPFNENTRY3
{
    struct
    {
        unsigned char Priority : 3;
        unsigned char OnProtectedStandby : 1;
        unsigned char InPageError : 1;
        unsigned char SystemChargedPage : 1;
        unsigned char RemovalRequested : 1;
        unsigned char ParityError : 1;
    };
} MMPFNENTRY3, * PMMPFNENTRY3;
typedef struct _MI_PFN_ULONG5
{
    union
    {
        unsigned long EntireField;
        struct
        {
            struct
            {
                unsigned long NodeBlinkHigh : 21;
                unsigned long NodeFlinkMiddle : 11;
            };
        } StandbyList;
        struct
        {
            unsigned char ModifiedListBucketIndex : 4;
        } MappedPageList;
        struct
        {
            struct
            {
                unsigned char AnchorLargePageSize : 2;
                unsigned char Spare1 : 6;
            };
            unsigned char ViewCount;
            unsigned short Spare2;
        } Active;
    };
} MI_PFN_ULONG5, * PMI_PFN_ULONG5;
typedef struct _MMPFN
{
    union
    {
        struct _LIST_ENTRY ListEntry;
        struct _RTL_BALANCED_NODE TreeNode;
        struct
        {
            union
            {
                union
                {
                    struct _SINGLE_LIST_ENTRY NextSlistPfn;
                    void* Next;
                    struct
                    {
                        unsigned __int64 Flink : 40;
                        unsigned __int64 NodeFlinkLow : 24;
                    };
                    struct _MI_ACTIVE_PFN Active;
                };
            } u1;
            union
            {
                struct _MMPTE* PteAddress;
                unsigned __int64 PteLong;
            };
            struct _MMPTE OriginalPte;
        };
    };
    struct _MIPFNBLINK u2;
    union
    {
        union
        {
            struct
            {
                unsigned short ReferenceCount;
                struct _MMPFNENTRY1 e1;
                struct _MMPFNENTRY3 e3;
            };
            struct
            {
                unsigned short ReferenceCount;
            } e2;
            struct
            {
                unsigned long EntireField;
            } e4;
        };
    } u3;
    struct _MI_PFN_ULONG5 u5;
    union
    {
        union
        {
            struct
            {
                unsigned __int64 PteFrame : 40;
                unsigned __int64 ResidentPage : 1;
                unsigned __int64 Unused1 : 1;
                unsigned __int64 Unused2 : 1;
                unsigned __int64 Partition : 10;
                unsigned __int64 FileOnly : 1;
                unsigned __int64 PfnExists : 1;
                unsigned __int64 NodeFlinkHigh : 5;
                unsigned __int64 PageIdentity : 3;
                unsigned __int64 PrototypePte : 1;
            };
            unsigned __int64 EntireField;
        };
    } u4;
} MMPFN, * PMMPFN;


#endif MMTYPES