// alienfan-tools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "acpilib.h"
#include "acpi.h"

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
    // PCPT/???? - CPU internal
    // CUPT/SEN1 - CPU external
    // TH1R/SEN2 - GPU internal
    // GRAP/SEN3 - GPU external
    // TH0R/SEN4 - SSD
    // TH0L/SEN5 - Ambient(!)

    ACPI_EVAL_OUTPUT_BUFFER* res[6], *resName;
    USHORT nType[6];
    USHORT nsType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_SEN1_STR"));
    string senNames[6];
    senNames[0] = "CPU Internal Thermistor";
    for (int i = 1; i < 6; i++) {
        TCHAR keyname[] = TEXT("\\____SB_PCI0LPCBEC0_SEN1_STR");
        keyname[23] = i + '0';
        if (resName = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(keyname, &nsType)) {
            char* c_name = new char[resName->Argument[0].DataLength + 1];
            wcstombs_s(NULL, c_name, resName->Argument[0].DataLength, (TCHAR*) resName->Argument[0].Data, resName->Argument[0].DataLength);
            senNames[i] = c_name;
            delete[] c_name;
        }
    }
    nType[0] = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_PCPT"));
    nType[1] = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_CUPT"));
    nType[2] = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH1R"));
    nType[3] = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_GRAP"));
    nType[4] = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH0R"));
    nType[5] = GetNSType((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH0L"));
    res[0] = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_PCPT"), &nType[0]);
    res[1] = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_CUPT"), &nType[1]);
    res[2] = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH1R"), &nType[2]);
    res[3] = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_GRAP"), &nType[3]);
    res[4] = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH0R"), &nType[4]);
    res[5] = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH0L"), &nType[5]);
    if (res[0] && res[1] && res[2] && res[3] && res[4] && res[5]) {
        cout << "Temperatures:" << endl;
        for (int i = 0; i < 6; i++)
            cout << senNames[i] << ": " << res[i]->Argument[0].Argument << endl;
    }


}

void Usage() {
    cout << "Usage: alienfan-cli [command[=value{,value}] [command...]]" << endl
        << "Avaliable commands: " << endl
        << "usage, help\t\tShow this usage" << endl
        << "dump\t\t\tDump all ACPI values avaliable (for debug and new hardware support)" << endl
        << "rpm\t\t\tShow fan(s) RPMs" << endl
        << "temp\t\t\tShow known temperature sensors values" << endl
        << "unlock=<value>\t\tUnclock fan controls (1 - unlock, 0 - lock)" << endl
        << "boost=<fan1>,<fan2>\tSet fans RPM boost level (0..100 - in percent)" << endl;
}

int main(int argc, char* argv[])
{
    std::cout << "AlienFan-cli v0.0.2.0\n";

    HANDLE acc = OpenAcpiService();

    if (argc < 2) 
    {
        Usage();
        return 1;
    }

    if (acc && QueryAcpiNSInLib()) {
        ACPI_EVAL_OUTPUT_BUFFER* res, * res2;
        USHORT nType = ACPI_TYPE_INTEGER;
        for (int cc = 1; cc < argc; cc++) {
            string arg = string(argv[cc]);
            size_t vid = arg.find_first_of('=');
            string command = arg.substr(0, vid);
            string values;
            vector<string> args;
            if (vid != string::npos) {
                size_t vpos = 0;
                values = arg.substr(vid + 1, arg.size());
                while (vpos < values.size()) {
                    size_t tvpos = values.find(',', vpos);
                    args.push_back(values.substr(vpos, tvpos - vpos));
                    vpos = tvpos == string::npos ? values.size() : tvpos + 1;
                }
            }
            //cerr << "Executing " << command << " with " << values << endl;
            if (command == "usage" || command == "help" || command == "?") {
                Usage();
                continue;
            }
            if (command == "rpm") {
                GetRPM();
                continue;
            }
            if (command == "temp") {
                GetTemp();
                continue;
            }
            if (command == "dump") {
                ACPI_NS_DATA data = {0};
                QueryAcpiNS(acc, &data, 0xc1);
                DumpAcpi(data);
                continue;
            }
            if (command == "unlock") {
                int lock = !atoi(args[0].c_str());
                if (lock)
                    cout << "Locking device...";
                else
                    cout << "Unlocking device...";
                res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_IETMMCHG"), (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, lock));
                cout << " Done." << endl;
                continue;
            }
            if (command == "boost") {
                if (args.size() < 2) {
                    cout << "Boost: incorrect arguments" << endl;
                    continue;
                }
                UINT boost1 = atoi(args[0].c_str()), boost2 = atoi(args[1].c_str());
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM1"), &nType);
                res2 = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM2"), &nType);
                if (res && res2)
                    cout << "Current boost for FAN1 - " << (UINT) res->Argument[0].Argument << ", FAN2 - " << res2->Argument[0].Argument << endl;
                else {
                    cout << "Can't get current boost. Driver error?" << endl;
                    continue;
                }
                ACPI_EVAL_INPUT_BUFFER_COMPLEX* args, * args2;
                args = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0x32);
                args = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(args, boost1);
                args2 = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0x33);
                args2 = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(args2, boost2);
                res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_AMW1SRPM"), args);
                res2 = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput((TCHAR*) TEXT("\\____SB_AMW1SRPM"), args2);
                if (res && res2) {
                    cout << "Set boost done with (" << hex << res->Argument[0].Argument << "," << res2->Argument[0].Argument << dec << ")" << endl;
                }
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM1"), &nType);
                res2 = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_AMW1RPM2"), &nType);
                if (res && res2)
                    cout << "New boost for FAN1 - " << (UINT) res->Argument[0].Argument << ", FAN2 - " << res2->Argument[0].Argument << endl;
                continue;
            }
            cout << "Unknown command - " << command << ", use \"usage\" or \"help\" for information" << endl;
        }
    } else {
        cout << "Can't load driver or access denied. Are you in Test mode and admin?" << endl;
    }

    cout << "Done!" << endl;
}
