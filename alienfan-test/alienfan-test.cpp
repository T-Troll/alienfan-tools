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

    if (!GetServiceName(driverLocation, MAX_PATH)) {
        return 0;
    }

    schSCManager = OpenSCManager(
        NULL,                   // local machine
        NULL,                   // local database
        SC_MANAGER_ALL_ACCESS   // access required
    );

    if (!schSCManager) {
        return 0;
    }

    InstallService(schSCManager, driverLocation);
    DemandService(schSCManager);

    driver = OpenAcpiDevice();
    
    if (driver) {
   
        EvalAcpiMethod(driver, "\\_SB.PCI0.LPCB.EC0.SEN1._TMP", (PVOID*)&res);
        if (res)
            cout << "Temp - " << (res->Argument[0].Argument - 0xaac) / 0xa << endl;


        BYTE operand[4] = {0xc, 0x32, 0, 0};
        args = (PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX) PutIntArg(NULL, 0);
        args = (PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX) PutIntArg(args, 0x14);
        args = (PACPI_EVAL_INPUT_BUFFER_COMPLEX_EX) PutBuffArg(args, 4, operand);

        EvalAcpiMethodArgs(driver, "\\_SB.AMW1.WMAX", args, (PVOID *) &res);
        if (res)
            cout << "Boost - " << res->Argument[0].Argument << endl;

        CloseAcpiDevice(driver);
    }

    StopService(schSCManager);
    RemoveService(schSCManager);
    CloseServiceHandle(schSCManager);

    std::cout << "Hello World!\n";
}
