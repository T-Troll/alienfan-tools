// alienfan-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "alienfan-low.h"

BOOLEAN
InstallService(
    __in SC_HANDLE SchSCManager,
    __in LPTSTR    ServiceName,
    __in LPTSTR    ServiceExe
)
/*++

Routine Description:

create a on-demanded service 

Arguments:

SchSCManager    - Handle of service control manager
ServiceName     - Service Name to create
ServiceExe      - Service Excute file path

Return Value:

TRUE            - Service create and installed succesfully
FALSE           - Failed to create service

--*/
{
    SC_HANDLE   schService;
    DWORD       err;

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //

    //
    // Create a new a service object.
    //

    schService = CreateService(SchSCManager,           // handle of service control manager database
                               ServiceName,            // address of name of service to start
                               ServiceName,            // address of display name
                               SERVICE_ALL_ACCESS,     // type of access to service
                               SERVICE_USER_OWN_PROCESS, //SERVICE_KERNEL_DRIVER,  // type of service
                               SERVICE_DEMAND_START,   // when to start service
                               SERVICE_ERROR_NORMAL,   // severity if service fails to start
                               ServiceExe,             // address of name of binary file
                               NULL,                   // service does not belong to a group
                               NULL,                   // no tag requested
                               NULL,                   // no dependency names
                               NULL,                   // use LocalSystem account
                               NULL                    // no password for service account
    );

    if (schService == NULL) {

        err = GetLastError();

        if (err == ERROR_DUPLICATE_SERVICE_NAME || err == ERROR_SERVICE_EXISTS) {
            //
            // Ignore this error.
            //ApiError(err,_T("Service already installed"));
            //
            return TRUE;
        } else {
            //
            // Indicate an error.
            //
            //ApiError(err,_T("InstallService"));
            return  FALSE;
        }
    }

    //
    // Close the service object.
    //
    if (schService) {

        CloseServiceHandle(schService);
    }

    //
    // Indicate success.
    //
    return TRUE;
}

BOOLEAN
RemoveService(
    __in SC_HANDLE SchSCManager,
    __in LPTSTR    ServiceName
)
/*++

Routine Description:

Remove service 

Arguments:

SchSCManager    - Handle of service control manager
ServiceName     - Service Name to remove    

Return Value:

TRUE            - Remove Service Successfully
FALSE           - Failed to Remove Service

--*/
{
    SC_HANDLE   schService;
    BOOLEAN     rCode;

    //
    // Open the handle to the existing service.
    //

    schService = OpenService(
        SchSCManager,
        ServiceName,
        SERVICE_ALL_ACCESS
    );

    if (schService == NULL) {
        //ApiError(GetLastError(), TEXT("RemoveService failed!"));
        //
        // Indicate error.
        //
        return FALSE;
    }

    //
    // Mark the service for deletion from the service control manager database.
    //
    if (DeleteService(schService)) {

        //
        // Indicate success.
        //
        rCode = TRUE;

    } else {
        //ApiError(GetLastError(), _T("DeleteService failed"));
        //
        // Indicate failure.  Fall through to properly close the service handle.
        //
        rCode = FALSE;
    }

    //
    // Close the service object.
    //
    if (schService) {

        CloseServiceHandle(schService);
    }

    return rCode;

}  

BOOLEAN
DemandService(
    __in SC_HANDLE SchSCManager,
    __in LPTSTR    ServiceName
)
/*++

Routine Description:

Start service on demanded

Arguments:

SchSCManager    - Handle of service control manager
ServiceName     - Service Name to start    

Return Value:

TRUE            - Start Service Successfully
FALSE           - Failed to Start Service

--*/
{
    SC_HANDLE   schService;
    DWORD       err;

    //
    // Open the handle to the existing service.
    //

    schService = OpenService(SchSCManager,
                             ServiceName,
                             SERVICE_ALL_ACCESS
    );

    if (schService == NULL) {

        //printf("OpenService failed!  Error = %d \n", GetLastError());
        //ApiError(GetLastError(), _T("DemainService failed!"));
        //
        // Indicate failure.
        //

        return FALSE;
    }

    //
    // Start the execution of the service (i.e. start the driver).
    //

    if (!StartService(schService,     // service identifier
                      0,              // number of arguments
                      NULL            // pointer to arguments
    )) {

        err = GetLastError();

        if (err == ERROR_SERVICE_ALREADY_RUNNING) {

            //
            // Ignore this error.
            //

            return TRUE;

        } else {

            //printf("StartService failure! Error = %d \n", err );
            //ApiError(err, _T("StartService failed"));

            //
            // Indicate failure.  Fall through to properly close the service handle.
            //

            return FALSE;
        }

    }

    //
    // Close the service object.
    //

    if (schService) {

        CloseServiceHandle(schService);
    }

    return TRUE;

} 



BOOLEAN
StopService(
    __in SC_HANDLE SchSCManager,
    __in LPTSTR    ServiceName
)
/*++

Routine Description:

Start service on demanded

Arguments:

SchSCManager    - Handle of service control manager
ServiceName     - Service Name to Stop    

Return Value:

TRUE            - Start Service Successfully
FALSE           - Failed to Start Service

--*/
{
    BOOLEAN         rCode = TRUE;
    SC_HANDLE       schService;
    SERVICE_STATUS  serviceStatus;

    //
    // Open the handle to the existing service.
    //

    schService = OpenService(SchSCManager,
                             ServiceName,
                             SERVICE_ALL_ACCESS
    );

    if (schService == NULL) {

        //printf("OpenService failed!  Error = %d \n", GetLastError());
        //ApiError(GetLastError(), _T("StopService failed!"));
        return FALSE;
    }

    //
    // Request that the service stop.
    //

    if (ControlService(schService,
                       SERVICE_CONTROL_STOP,
                       &serviceStatus
    )) {

        //
        // Indicate success.
        //

        rCode = TRUE;

    } else {

        //printf("ControlService failed!  Error = %d \n", GetLastError() );
        //ApiError(GetLastError(), _T("ControlService failed"));

        //
        // Indicate failure.  Fall through to properly close the service handle.
        //

        rCode = FALSE;
    }

    //
    // Close the service object.
    //

    if (schService) {

        CloseServiceHandle (schService);
    }

    return rCode;

}

BOOLEAN
GetServiceName(
    __inout_bcount_full(BufferLength) TCHAR *DriverLocation,
    __in ULONG BufferLength
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
    HANDLE  fileHandle = NULL;
    TCHAR   driver[MAX_PATH];
    TCHAR   file[MAX_PATH];
    size_t  pcbLength = 0;
    size_t  Idx;

    if (DriverLocation == NULL || BufferLength < 1) {
        return FALSE;
    }

    if (GetSystemDirectory(DriverLocation, BufferLength - 1) == 0) {
        return FALSE;
    }
    if (GetCurrentDirectory(MAX_PATH, driver) == 0)
    {
        return FALSE;
    }
    GetModuleFileName (NULL, file, MAX_PATH - 1);

    if (FAILED(StringCbLength(file, MAX_PATH, &pcbLength)) || pcbLength == 0) {
        return FALSE;
    }
    pcbLength = pcbLength / sizeof(TCHAR);
    for (Idx = (pcbLength) - 1; Idx > 0; Idx --) {
        if (file[Idx] == '\\') {
            file[Idx + 1]  = 0;
            break;
        }
    }

    if (FAILED(StringCbCat(file, MAX_PATH, _T("HwAccU.dll")))) {
        return FALSE;
    }

    StringCbPrintf (DriverLocation,BufferLength, file);

    // test file is existing
    if ((fileHandle = CreateFile(file,
                                 GENERIC_READ,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL
    )) == INVALID_HANDLE_VALUE) {

        //
        // Indicate failure.
        //
        return FALSE;
    }


    //Close open file handle.

    if (fileHandle) {

        CloseHandle(fileHandle);
    }
    //MessageBox (NULL, file, file, MB_OK);

    //
    // Setup path name to driver file.
    //
    //if (FAILED(StringCbCat(driver, MAX_PATH, _T("\\HwAcc.sys")))) {
    //    return FALSE;
    //}

    /*if (GetLastError () == 0) {

    StringCbCat (DriverLocation, MAX_PATH, _T("\\HwAcc.sys"));        
    }*/    

    return TRUE;
}

int main()
{
    HANDLE driver = NULL;
    //driver = OpenAcpiDevice();
    SC_HANDLE   schSCManager = NULL;
    TCHAR  driverLocation[MAX_PATH] = {0};

    //EvalAcpiMethod(driver, "", NULL);
    //CloseAcpiDevice(driver);

    GetServiceName(driverLocation, MAX_PATH);

    schSCManager = OpenSCManager(
        NULL,                   // local machine
        NULL,                   // local database
        SC_MANAGER_ALL_ACCESS   // access required
    );

    if (schSCManager) {
        if (InstallService(schSCManager,
                           (LPTSTR)_T("HwAccU"),
                           driverLocation
        )) {
            DemandService(
                schSCManager,
                (LPTSTR)_T("HwAccU")
            );
            // call wiil be here
            driver = CreateFile(
                _T("\\\\.\\HwAccU"), //_T("\\\\?\\GLOBALROOT\\Device\\HwAccU"), 
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
            // stop now...
            StopService(schSCManager, (LPTSTR)_T("HwAccU"));
            RemoveService(schSCManager, (LPTSTR)_T("HwAccU"));
        }
        CloseServiceHandle(schSCManager);
    }
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
