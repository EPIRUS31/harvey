#ifndef CONFIG_H
#define CONFIG_H

#include "include.h"

namespace KGLOBAL
{
	UNICODE_STRING DeviceName;
	UNICODE_STRING SymbolicName;
	UNICODE_STRING CustomSymbolicName;

	BOOLEAN BlockDebugging = false;
	BOOLEAN ManualMapped;
}

#endif CONFIG_H