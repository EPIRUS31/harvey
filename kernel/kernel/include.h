#ifndef INCLUDE_H
#define INCLUDE_H

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <ntdef.h>
#include <ntstrsafe.h>
#include <intrin.h>
#include <stdarg.h> 
#include <ntstrsafe.h>

#include "mmtypes.h"

EXTERN_C PLIST_ENTRY PsLoadedModuleList;
EXTERN_C PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);


#endif // !INCLUDE_H
