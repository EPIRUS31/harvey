#ifndef CONFIG_H
#define CONFIG_H

#include "include.h"

namespace KGLOBAL
{
	inline UNICODE_STRING DeviceName = { 0 };
	inline UNICODE_STRING SymbolicName = { 0 };
	inline UNICODE_STRING CustomSymbolicName = { 0 };
	 
	inline BOOLEAN BlockDebugging = false;
	inline BOOLEAN ManualMapped;
}

#endif CONFIG_H