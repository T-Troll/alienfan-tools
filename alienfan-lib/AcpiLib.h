/*++

2008-2020  NickelS

Module Name:

    AcpiLib.h

Abstract:

    Acpi lib definition file

Environment:

    User mode only.

--*/

#ifndef _ACPI_LIB_H_
#define _ACPI_LIB_H_
#define METHOD_START_INDEX  0xC1
#include <ctype.h>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <Setupapi.h>
#include <devioctl.h>
#include "../HwAcc/Ioctl.h"
#include "mdmap.h"

//#define FIELD_OFFSET(type, field) (ULONG_PTR)(&((type *)0)->field)

//#define EXPORT_DLL #ifdef ACPILIB_EXPORTS __declspec(dllexport) #else __declspec(dllimport) #endif

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef ACPILIB_EXPORTS
//#define ACPI_LIB_FUNCTION __declspec(dllexport)
//#else
//#define ACPI_LIB_FUNCTION __declspec(dllimport)
//#endif

#define ACPI_LIB_FUNCTION 

ACPI_LIB_FUNCTION
void
APIENTRY
SetBinaryID(int);

ACPI_LIB_FUNCTION
VOID
APIENTRY
CloseDll();

ACPI_LIB_FUNCTION
VOID
APIENTRY
CloseAcpiService(
    HANDLE hDriver
);

ACPI_LIB_FUNCTION
HANDLE
APIENTRY
OpenAcpiService(
    );

ACPI_LIB_FUNCTION
BOOLEAN
APIENTRY
OpenAcpiDevice(
    __in HANDLE hDriver
);

ACPI_LIB_FUNCTION
BOOL
APIENTRY
LoadNotifiyMethod(
    HANDLE      hDriver,
    AML_SETUP* pAmlSetup,
    ULONG       uSize
);

ACPI_LIB_FUNCTION
void
APIENTRY
UnloadNotifiyMethod(
    HANDLE      hDriver
);

ACPI_LIB_FUNCTION
BOOL
APIENTRY
EvalAcpiNS(
    HANDLE          hDriver,
    ACPI_NAMESPACE* pAcpiName,
    PVOID* pReturnData,
    ULONG* puLength
);

ACPI_LIB_FUNCTION
BOOL
APIENTRY
EvalAcpiNSArg(
    HANDLE          hDriver,
    PACPI_METHOD_ARG_COMPLEX pComplexData,
    PVOID* pReturnData,
    UINT Size
);

ACPI_LIB_FUNCTION
PVOID
APIENTRY
EvalAcpiNSArgOutput(
    TCHAR* pPath,
    ACPI_EVAL_INPUT_BUFFER_COMPLEX* pComplexInput
);

ACPI_LIB_FUNCTION
PVOID
APIENTRY
EvalAcpiNSOutput(
    TCHAR* pPath
);

ACPI_LIB_FUNCTION
PVOID
APIENTRY
ReadAcpiMemory(
    HANDLE hDriver,
    PVOID  Address,
    ULONG  Size
);

ACPI_LIB_FUNCTION
BOOLEAN
APIENTRY
QueryAcpiNS(
    HANDLE          hDriver,
    ACPI_NS_DATA*   pAcpiNsData,
    UINT            MethodStartIndex
);

ACPI_LIB_FUNCTION
ACPI_METHOD_MAP*
APIENTRY
GetMethod(
    UINT32 NameSeg
);

ACPI_LIB_FUNCTION
void
APIENTRY
ReleaseAcpiNS(
);


ACPI_LIB_FUNCTION
BOOLEAN
APIENTRY
QueryAcpiNSInLib(
);

ACPI_LIB_FUNCTION
void
APIENTRY
SaveAcpiObjects(
    TCHAR* chFile
);

ACPI_LIB_FUNCTION
void
APIENTRY
LoadAcpiObjects(
    TCHAR* chFile
);

ACPI_LIB_FUNCTION
UINT
APIENTRY
GetNamePath(
    UINT32* puParent,
    BYTE* pChild
);

ACPI_LIB_FUNCTION
int
APIENTRY
GetNamePathFromPath(
    TCHAR* puParent,
    TCHAR* puChild
);

ACPI_LIB_FUNCTION
USHORT
APIENTRY
GetNameType(
    TCHAR* pParent
);

ACPI_LIB_FUNCTION
BOOLEAN
APIENTRY
GetNameIntValue(
    TCHAR* pParent,
    ULONG64* uLong64
);

ACPI_LIB_FUNCTION
int
APIENTRY
GetNameStringValue(
    TCHAR* pParent,
    TCHAR* pString
);

ACPI_LIB_FUNCTION
int
APIENTRY
GetNameAddrFromPath(
    TCHAR* pParent,
    PVOID* pChild
);


ACPI_LIB_FUNCTION
PVOID
APIENTRY
GetNameAddr(
    TCHAR* pParent
);

//ACPI_LIB_FUNCTION
//void
//APIENTRY
//GetNameFromAddr(
//    ACPI_NS* pAcpiNS,
//    TCHAR* pName
//);

ACPI_LIB_FUNCTION
UINT64
APIENTRY
AslFromPath(
    TCHAR* pPath,
    TCHAR* pAsl
);

ACPI_LIB_FUNCTION
UINT64
APIENTRY
EvalAcpiNSAndParse(
    TCHAR* pPath,
    TCHAR* pAsl
);

ACPI_LIB_FUNCTION
BOOLEAN
APIENTRY
GetArgsCount(
    TCHAR* pParent,
    ULONG64* uLong64
);

ACPI_LIB_FUNCTION
PVOID
PutBuffArg(
    PVOID   pArgs,
    UINT    Length,
    UCHAR* pBuf
);

ACPI_LIB_FUNCTION
PVOID
PutStringArg(
    PVOID   pArgs,
    UINT    Length,
    TCHAR*  pString
);

ACPI_LIB_FUNCTION
PVOID
PutIntArg(
    PVOID   pArgs,
    UINT64  value
);

ACPI_LIB_FUNCTION
UINT64
APIENTRY
EvalAcpiNSArgAndParse(
    TCHAR* pPath,
    ACPI_EVAL_INPUT_BUFFER_COMPLEX *pComplexInput,
    TCHAR* pAsl
);

ACPI_LIB_FUNCTION
void
APIENTRY
FreeMemory(
    PVOID pMem
);

ACPI_LIB_FUNCTION
BOOLEAN
APIENTRY
NotifyDevice(
	TCHAR* pchPath,
	ULONG	ulCode
);

ACPI_LIB_FUNCTION
int
APIENTRY
GetNSType(
    TCHAR* pchPath
);

ACPI_LIB_FUNCTION
VOID *
APIENTRY
GetNSValue(
    TCHAR* pchPath,
    USHORT *pulLength
);

ACPI_LIB_FUNCTION
VOID*
APIENTRY
GetRawData(
    TCHAR* pchPath,
    USHORT* puType,
    ULONG* puLength
);

#ifdef __cplusplus
}
#endif

#endif

