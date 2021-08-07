// alienfan-tools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "acpilib.h"
#include "acpi.h"

//#pragma comment(lib, "AcpiLib.lib")

using namespace std;

void DumpAcpi(ACPI_NS_DATA data) {
    int offset = 0;
    string fullname;
    ACPI_NAMESPACE* curData = data.pAcpiNS;
    for (int i = 0; i < data.uCount; i++) {
        //for (int j = 0; j < offset; j++)
        //    cout << "|";
        string mname = (char*)curData->MethodName;
        mname.resize(4);
        cout << fullname << mname << endl;
        if (curData->pChild) {
            // need to go deep...
            if (offset > 0)
                fullname += mname;
            curData = curData->pChild;
            offset++;
        } else
            while (curData && !curData->pNext) {
                // going up to tree
                curData = curData->pParent;
                offset--;
                if (offset > 0)
                    fullname.resize((offset -1)* 4);
            }
            if (curData) {
                curData = curData->pNext;
            } else
                break;
    }
}

void GetRPM() {
    ACPI_EVAL_OUTPUT_BUFFER* res, *res2;
    res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_AMW1FNSR"), (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0x32));
    if (res)
        cout << "RPM1 is " << res->Argument[0].Argument << ", ";
    res2 = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_AMW1FNSR"), (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0x33));
    if (res2)
        cout << "RPM2 is " << res2->Argument[0].Argument << endl;
}

void GetTemp() { 
    ACPI_EVAL_OUTPUT_BUFFER* res, *res2;
    USHORT nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_PCPT"));
    res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_PCPT"), &nType); // PCPT
    res2 = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_CUPT"), &nType);
    if (res && res2) {
        cout << "PCB temp " << res->Argument[0].Argument << ", GPU temp " << res2->Argument[0].Argument << endl;
    }


}

void Usage() {
    cout << "Usage: alienfan-cli [lock [boost1 boost2]]" << endl
        << "\tLock - 1 off, 0 on" << endl
        << "\tBoost - 0..100 (in percent)" << endl
        << "Running without parameters reveal current fans RPMs" << endl;
}

int main(int argc, char* argv[])
{
    std::cout << "AlienFan-cli v0.0.1.0\n";

    ACPI_NS_DATA data = {0};
    HANDLE acc = OpenAcpiService();
    if (acc) {
        OpenAcpiDevice(acc);
        QueryAcpiNS(acc, &data, 0xc1);
        ACPI_EVAL_OUTPUT_BUFFER* res, *res2;
        USHORT nType = ACPI_TYPE_INTEGER;
        switch (argc) {
        case 1: // show RPM only
            GetRPM();
            GetTemp();
            break;
        case 2:
        {// lock/unlock only
            int lock = !atoi(argv[1]);
            if (argv[1][0] == '?' || argv[1][0] == '-' || argv[1][0] == '/' || lock < 0 || lock > 1) {
                Usage();
                break;
            }
            cout << "Setting lock " << lock << endl;
            res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_IETMMCHG"), (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, lock));
            //if (res)
            //    cout << "Lock Result " << hex << res->Argument[0].Argument << endl;
            //else
            //    cout << "Unlock operation failed." << endl;
        } break;
        case 4:
        {// lock/unlock and set boost
            UINT lock = !atoi(argv[1]);
            UINT boost1 = atoi(argv[2]), boost2 = atoi(argv[3]);
            if (argv[1][0] == '?' || argv[1][0] == '-' || argv[1][0] == '/' 
                || boost1 < 0 || boost1 > 100
                || boost2 < 0 || boost2 > 100
                || lock < 0 || lock > 1) {
                Usage();
                break;
            }
            cout << "Setting lock " << lock << endl;
            res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_IETMMCHG"), (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, lock));
            //if (res)
            //    cout << "Lock Result " << hex << res->Argument[0].Argument << dec << endl;
            //else
            //    cout << "Unlock operation failed." << endl;
            res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM1"), &nType);
            res2 = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM2"), &nType);
            if (res && res2)
                cout << "Current boost for FAN1 " << (UINT) res->Argument[0].Argument << ", FAN2 " << res2->Argument[0].Argument << endl;
            ACPI_EVAL_INPUT_BUFFER_COMPLEX* args, * args2;
            args = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0x32);
            args = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(args, boost1);
            args2 = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0x33);
            args2 = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(args2, boost2);
            res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_AMW1SRPM"), args);
            res2 = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_AMW1SRPM"), args2);
            if (res && res2) {
                cout << "Set boost for fans result " << hex << res->Argument[0].Argument << "," << res2->Argument[0].Argument << dec << endl;
            }
            res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM1"), &nType);
            res2 = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM2"), &nType);
            if (res && res2)
                cout << "New boost for FAN1 " << (UINT) res->Argument[0].Argument << ", FAN2 " << res2->Argument[0].Argument << endl;
        } break;
        default: // usage
            Usage();
        }
    } else {
        cout << "Can't load driver. Are you in Test mode and admin?" << endl;
    }

    cout << "Done!" << endl;
    //CloseAcpiService(acc);
}
