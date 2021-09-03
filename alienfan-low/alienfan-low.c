#include "alienfan-low.h"
#include <acpiioct.h>
#include <stdlib.h>

#pragma comment(lib,"setupapi.lib")

TCHAR           DevInstanceId[201];
TCHAR           DevInstanceId1[201];

BOOL
GetAcpiDevice (
    PCTSTR Name, 
    LPBYTE PropertyBuffer, 
    DWORD Idx
)
/*++

Routine Description:

Get the current full path service name for install/stop/remove service

Arguments:

DriverLocation  - Buffer to receive location of service
BufferLength    - Buffer size

Return Value:

TRUE            - Get service full path successfully
FALSE           - Failed to get service full path

--*/
{
    HDEVINFO    hdev;
    SP_DEVINFO_DATA devdata;    
    DWORD       PropertyRegDataType;
    DWORD       RequiedSize;

    hdev = SetupDiGetClassDevsEx (NULL, Name, NULL, DIGCF_ALLCLASSES, NULL, NULL, NULL);

    if (hdev != INVALID_HANDLE_VALUE) {
        if (TRUE) {
            ZeroMemory (&devdata, sizeof (devdata));
            devdata.cbSize = sizeof (devdata);      
            if (SetupDiEnumDeviceInfo (hdev, Idx, &devdata)) {      
                if (SetupDiGetDeviceInstanceId (hdev, & devdata, &DevInstanceId[0], 200, NULL)) {
                    CopyMemory (DevInstanceId1, DevInstanceId, 201);
                    if (SetupDiGetDeviceRegistryProperty (hdev, & devdata, 0xE, 
                                                          &PropertyRegDataType, &PropertyBuffer[0],0x400, &RequiedSize)) {
                        //printf (PropertyBuffer);                 
                        //printf ("\n");
                    } else {
                        //printf ("Failed to call SetupDiGetDeviceRegistryProperty\n");
                    }
                } else {
                    //printf ("Failed to call SetupDiGetDeviceInstanceId\n");
                }
            } else {
                SetupDiDestroyDeviceInfoList (hdev);    
                //printf ("Failed to call SetupDiEnumDeviceInfo\n");
                return FALSE;
            }       
            SetupDiDestroyDeviceInfoList (hdev);    
        }
    } else {

        //printf ("Failed to call SetupDiGetClassDevsEx\n");
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
APIENTRY
EvalAcpiMethod(
    __in HANDLE hDriver,
    __in const char* puNameSeg,
    __in PVOID outputBuffer
) {
    BOOL                     IoctlResult;
    DWORD                       ReturnedLength, LastError;

    PACPI_EVAL_INPUT_BUFFER_EX      pMethodWithoutInputEx;
    ACPI_EVAL_OUTPUT_BUFFER     outbuf;

   pMethodWithoutInputEx = (PACPI_EVAL_INPUT_BUFFER_EX)  malloc (sizeof (ACPI_EVAL_INPUT_BUFFER_EX));
		//(ACPI_EVAL_INPUT_BUFFER_EX*)ExAllocatePoolWithTag(0, sizeof(ACPI_EVAL_INPUT_BUFFER_EX), MY_TAG);

	pMethodWithoutInputEx->Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE_EX;
    strcpy(pMethodWithoutInputEx->MethodName, "\\_SB_.PCI0.LPCB.EC0_.SEN1._TMP");
	//pMethodWithoutInputEx->MethodName[0] = puNameSeg[0];
	//pMethodWithoutInputEx->MethodName[1] = puNameSeg[1];
	//pMethodWithoutInputEx->MethodName[2] = puNameSeg[2];
	//pMethodWithoutInputEx->MethodName[3] = puNameSeg[3];
	//pMethodWithoutInputEx->MethodName[4] = 0;

    IoctlResult = DeviceIoControl(
        hDriver,           // Handle to device
        IOCTL_ACPI_EVAL_METHOD_EX,    // IO Control code for Read
        pMethodWithoutInputEx,        // Buffer to driver.
        sizeof(ACPI_EVAL_INPUT_BUFFER_EX), // Length of buffer in bytes.
        &outbuf,     // Buffer from driver.
        sizeof(ACPI_EVAL_OUTPUT_BUFFER),
        &ReturnedLength,    // Bytes placed in DataBuffer.
        NULL                // NULL means wait till op. completes.
    );
    if (!IoctlResult)
        LastError = GetLastError();
    return (BOOLEAN) IoctlResult;
}

void
APIENTRY
CloseAcpiDevice(
    __in HANDLE hDriver
) {
    if (hDriver)
        CloseHandle(hDriver);
    hDriver = NULL;
}

HANDLE
APIENTRY
OpenAcpiDevice (
)
/*++

Routine Description:

Open Acpi Device

Arguments:

hDriver     - Handle of service

Return Value:

TRUE        - Open ACPI driver acpi.sys ready
FALSE       - Failed to open acpi driver

--*/ 
{
    HANDLE      hDriver = NULL;
    UINT        Idx;   
    BYTE        PropertyBuffer[0x401];
    TCHAR       *pChar;
    CHAR        AcpiName[0x401];
    BOOL        IoctlResult = FALSE;
    ACPI_NAME   acpi;
    ULONG       ReturnedLength = 0;
    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&osinfo)) {
        //  printf ("OS Major Version is %d, Minor Version is %d", osinfo.dwMajorVersion, osinfo.dwMinorVersion);
        return FALSE;
    }

    acpi.dwMajorVersion = osinfo.dwMajorVersion;
    acpi.dwMinorVersion = osinfo.dwMinorVersion;
    acpi.dwBuildNumber  = osinfo.dwBuildNumber;
    acpi.dwPlatformId   = osinfo.dwPlatformId;
    acpi.pAcpiDeviceName = PropertyBuffer;
    Idx = 0;    

    while (GetAcpiDevice (_T("ACPI_HAL"), PropertyBuffer, Idx)) {           
        if (Idx >= 1) {
            //break;
        }
        Idx ++;

        //if (sizeof(TCHAR) == sizeof(WCHAR)) {
            // unicode defined, need to change from unicode to uchar code..
            pChar = (TCHAR*)PropertyBuffer;
            WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK, pChar, 400, AcpiName, 400, NULL, NULL); 
            acpi.pAcpiDeviceName = AcpiName;
        //}

        acpi.uAcpiDeviceNameLength = (ULONG)strlen (acpi.pAcpiDeviceName);

        TCHAR base[256] = _T("\\\\?\\GLOBALROOT");
        wcscat(base, pChar);

        hDriver = CreateFile(
            base, // _T("\\\\?\\GLOBALROOT\\Device\\HWACC0"), 
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hDriver == INVALID_HANDLE_VALUE) {
            DWORD errNum = GetLastError();
            return hDriver;
        } else
            return hDriver;
        //IoctlResult = DeviceIoControl(
        //    hDriver,            // Handle to device
        //    (DWORD)IOCTL_GPD_OPEN_ACPI,    // IO Control code for Read
        //    &acpi,        // Buffer to driver.
        //    sizeof (ACPI_NAME), // Length of buffer in bytes.
        //    NULL,       // Buffer from driver.
        //    0,      // Length of buffer in bytes.
        //    &ReturnedLength,    // Bytes placed in DataBuffer.
        //    NULL                // NULL means wait till op. completes.
        //);
        //if (IoctlResult) {
        //    break;
        //}
    }

    return hDriver;
}