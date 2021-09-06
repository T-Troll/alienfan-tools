// alienfan-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;
#include "alienfan-low.h"
#include <acpiioct.h>

int main()
{
    HANDLE driver = NULL;
    SC_HANDLE   schSCManager = NULL;
    TCHAR  driverLocation[MAX_PATH] = {0};

    PACPI_EVAL_OUTPUT_BUFFER res = NULL;

    PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX args = NULL;

    //if (!GetServiceName(driverLocation, MAX_PATH)) {
    //    return 0;
    //}

    //schSCManager = OpenSCManager(
    //    NULL,                   // local machine
    //    NULL,                   // local database
    //    SC_MANAGER_ALL_ACCESS   // access required
    //);

    //if (!schSCManager) {
    //    return 0;
    //}

    //InstallService(schSCManager, driverLocation);
    //DemandService(schSCManager);


    cout << "Opening ACPI..." << endl;
    driver = OpenAcpiDevice();
    
    if (driver && driver != INVALID_HANDLE_VALUE) {

        cout << "Opening ACPI done!" << endl;
   
        cout << "Eval temp..." << endl;
        EvalAcpiMethod(driver, "\\_SB.PCI0.LPCB.EC0.SEN1._TMP", (PVOID*)&res);
        if (res)
            cout << "Temp - " << (res->Argument[0].Argument - 0xaac) / 0xa << endl;


        BYTE operand[4] = {0x5, 0x32, 0, 0};
        args = (PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX) PutIntArg(NULL, 0);
        args = (PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX) PutIntArg(args, 0x14);
        args = (PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX) PutBuffArg(args, 4, operand);
        cout << "Eval RPM..." << endl;
        EvalAcpiMethodArgs(driver, "\\_SB.AMW1.WMAX", args, (PVOID *) &res);
        if (res)
            cout << "RPM - " << res->Argument[0].Argument << endl;

        cout << "Closing ACPI..." << endl;
        CloseAcpiDevice(driver);
        cout << "Closing ACPI done!" << endl;
    } else
        cout << "Opening ACPI failed!" << endl;

    /*StopService(schSCManager);
    RemoveService(schSCManager);
    CloseServiceHandle(schSCManager);*/

    std::cout << "Goodbye!\n";
}
