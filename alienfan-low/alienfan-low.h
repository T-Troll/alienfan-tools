#pragma once
#include <ctype.h>
#include <windows.h>
#include <Setupapi.h>
#include <devioctl.h>
#include <tchar.h>
#include <strsafe.h>

#pragma warning(disable:4996)

typedef struct {
    PVOID   pAcpiDeviceName;
    ULONG   uAcpiDeviceNameLength;
    ULONG   dwMajorVersion;
    ULONG   dwMinorVersion;
    ULONG   dwBuildNumber;
    ULONG   dwPlatformId;
} ACPI_NAME;

#ifdef __cplusplus
extern "C" {
#endif

    HANDLE
        APIENTRY
        OpenAcpiDevice();

    void
        APIENTRY
        CloseAcpiDevice(
            __in HANDLE hDriver
        );

    BOOLEAN
        APIENTRY
        EvalAcpiMethod(
            __in HANDLE hDriver,
            __in const char* puNameSeg,
            __in PVOID outputBuffer
        );

#ifdef __cplusplus
}
#endif