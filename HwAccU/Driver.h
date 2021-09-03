/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    User-mode Driver Framework 2

--*/

#include <windows.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD HwAccUEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP HwAccUEvtDriverContextCleanup;

EXTERN_C_END

typedef struct {
    PVOID   pAcpiDeviceName;
    ULONG   uAcpiDeviceNameLength;
    ULONG   dwMajorVersion;
    ULONG   dwMinorVersion;
    ULONG   dwBuildNumber;
    ULONG   dwPlatformId;
} ACPI_NAME;
