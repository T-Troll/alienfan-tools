// alienfan-tools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "acpilib.h"
#include "acpi.h"

using namespace std;

// One thing to rule them all!
// AMW infewrface command - 3 parameters.
TCHAR* mainCommand = (TCHAR*) TEXT("\\____SB_AMW1WMAX");
// Last fan set
TCHAR* getLastSet = (TCHAR*) TEXT("\\____SB_AMW1RPM1");
// Additional temp sensor name pattern
TCHAR* tempNamePattern = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_SEN1_STR");
// Additional temp sensor value pattern
TCHAR* tempValuePattern = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_SEN1_TMP");

struct ALIENFAN_SEN_INFO {
    SHORT senIndex = 0;
    string name;
};

void DumpAcpi(ACPI_NS_DATA data) {
    int offset = 0;
    string fullname;
    ACPI_NAMESPACE* curData = data.pAcpiNS;
    for (UINT i = 0; i < data.uCount; i++) {
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


vector<ALIENFAN_SEN_INFO> ProbeTemp() {
    PACPI_EVAL_OUTPUT_BUFFER resName = NULL;
    SHORT nsType = 0;
    vector<ALIENFAN_SEN_INFO> res;
    ALIENFAN_SEN_INFO cur;
    for (int i = 0; i < 10; i++) {
        TCHAR keyname[] = TEXT("\\____SB_PCI0LPCBEC0_SEN1_STR");
        keyname[23] = i + '0';
        nsType = GetNSType(keyname);
        if (nsType != -1) {
            // Key found!
            if (resName = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(keyname, (USHORT*)&nsType)) {
                char* c_name = new char[resName->Argument[0].DataLength + 1];
                wcstombs_s(NULL, c_name, resName->Argument[0].DataLength, (TCHAR*) resName->Argument[0].Data, resName->Argument[0].DataLength);
                string senName = c_name;
                delete[] c_name;
                cur.senIndex = i;
                cur.name = senName;
                res.push_back(cur);
                //cout << "Sensor #" << i << ", Name: " << senName << endl;
            }
        }
    }
    return res;
}

int RunMainCommand(USHORT command, BYTE subcommand, BYTE arg1 = 0, BYTE arg2 = 0) {
    PACPI_EVAL_OUTPUT_BUFFER res = NULL;
    ACPI_EVAL_INPUT_BUFFER_COMPLEX* acpiargs;
    BYTE operand[4] = {subcommand, arg1, arg2, 0};
    //UINT operand = ((UINT) arg2) << 16 | (UINT) arg1 << 8 | subcommand;
    acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0);
    acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(acpiargs, command);
    acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutBuffArg(acpiargs, 4, operand);
    res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(mainCommand, acpiargs);
    if (res) {
        return (int) res->Argument[0].Argument;
    }
    return -1;
}

void Usage() {
    cout << "Usage: alienfan-cli [command[=value{,value}] [command...]]" << endl
        << "Avaliable commands: " << endl
        << "usage, help\t\tShow this usage" << endl
        << "dump\t\t\tDump all ACPI values avaliable (for debug and new hardware support)" << endl
        << "rpm\t\t\tShow fan(s) RPMs" << endl
        << "temp\t\t\tShow known temperature sensors values" << endl
        << "unlock\t\tUnclock fan controls" << endl
        << "power=<value>\t\tSet TDP to this level" << endl
        << "getfans\t\t\tShow current fan boost level (0..100 - in percent)" << endl
        << "setfans=<fan1>,<fan2>\tSet fans boost level (0..100 - in percent)" << endl
        << "  Power level can be 0 - 60W unlocked, 1 - 45W, 2 - 45W+GPU boost, 3 - 60W, 4 - 75W" << endl;
}

int main(int argc, char* argv[])
{
    std::cout << "AlienFan-cli v0.0.4\n";

    HANDLE acc = OpenAcpiService();

    if (argc < 2) 
    {
        Usage();
        return 1;
    }

    if (acc && QueryAcpiNSInLib()) {
        PACPI_EVAL_OUTPUT_BUFFER res;
        USHORT nsType = ACPI_TYPE_METHOD;

        // Checking hardware...
        nsType = GetNSType(mainCommand);
        if (nsType == ACPI_TYPE_METHOD) {
            cout << "Supported hardware detected!" << endl;

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
                    int prms = RunMainCommand(0x14, 0x5, 0x32);
                    if (prms >= 0)
                        cout << "CPU fan: " << prms;
                    prms = RunMainCommand(0x14, 0x5, 0x33);
                    if (prms >= 0)
                        cout << ", GPU fan: " << prms << endl;
                    else
                        cout << "RPM reading failed!" << endl;
                    //prms = RunMainCommand(0x14, 0x8, 0x32);
                    //if (prms >= 0)
                    //    cout << "Target CPU fan: " << prms;
                    //prms = RunMainCommand(0x14, 0x8, 0x33);
                    //if (prms >= 0)
                    //    cout << ", Target GPU fan: " << prms << endl;
                    continue;
                }
                if (command == "temp") {
                    // Probing sensors...
                    vector<ALIENFAN_SEN_INFO> sensors = ProbeTemp();
                    cout << sensors.size() << " additional temperature sensors detected." << endl;
                    // main CPU&GPU sensors from AWC!
                    int cTemp = RunMainCommand(0x14, 0x4, 1);
                    if (cTemp >= 0) {
                        cout << "CPU Internal Thermister: " << cTemp << endl;
                    }
                    cTemp = RunMainCommand(0x14, 0x4, 6);
                    if (cTemp >= 0) {
                        cout << "GPU Internal Thermister: " << cTemp << endl;
                    }
                    for (int i = 0; i < sensors.size(); i++) {
                        TCHAR keyname[] = TEXT("\\____SB_PCI0LPCBEC0_SEN1_TMP");
                        keyname[23] = sensors[i].senIndex + '0';
                        nsType = GetNSType(keyname);
                        res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(keyname, &nsType);
                        if (res)
                            cout << sensors[i].name << ": " << (res->Argument[0].Argument - 0xaac) / 0xa << endl;
                    }
                    continue;
                }
                if (command == "dump") {
                    ACPI_NS_DATA data = {0};
                    QueryAcpiNS(acc, &data, 0xc1);
                    DumpAcpi(data);
                    continue;
                }
                if (command == "unlock") {
                    int unlock = RunMainCommand(0x15, 0x1);
                    if (unlock >= 0)
                        cout << "Unlock result: " << unlock << endl;
                    else
                        cout << "Unlock failed!" << endl;
                    continue;
                }
                if (command == "power") {
                    if (args.size() < 1) {
                        cout << "Power: incorrect arguments" << endl;
                        continue;
                    }
                    BYTE unlockStage = atoi(args[0].c_str());
                    BYTE unlockType = 0;
                    switch (unlockStage) {
                    case 1: // 45W GPU boost
                        unlockType = 0xa3; break;
                    case 2: // 45W calm
                        unlockType = 0xa2; break;
                    case 3: // 60W
                        unlockType = 0xa0; break;
                    case 4: // 75W
                        unlockType = 0xa1; break;
                    }
                    int unlock = RunMainCommand(0x15, 0x1, unlockType);
                    if (unlock >= 0)
                        cout << "Power set result: " << unlock << endl;
                    else
                        cout << "Power set failed!" << endl;
                    continue;
                }
                if (command == "getfans") {
                    int prms = RunMainCommand(0x14, 0xc, 0x32);
                    if (prms >= 0)
                        cout << "CPU fan now at: " << prms;
                    prms = RunMainCommand(0x14, 0xc, 0x33);
                    if (prms >= 0)
                        cout << ", GPU fan now at: " << prms << endl;
                    else
                        cout << "Get fan settings failed!" << endl;
                    continue;
                }
                if (command == "setfans") {
                    if (args.size() < 2) {
                        cout << "Boost: incorrect arguments" << endl;
                        continue;
                    }
                    BYTE boost1 = atoi(args[0].c_str()), boost2 = atoi(args[1].c_str());
                    int prms = RunMainCommand(0x15, 0x2, 0x32, boost1);
                    if (prms >= 0)
                        cout << "CPU fan set to: " << (int)boost1;
                    prms = RunMainCommand(0x15, 0x2, 0x33, boost2);
                    if (prms >= 0)
                        cout << ", GPU fan set to: " << (int)boost2 << endl;
                    else
                        cout << "Set fan boost failed!" << endl;
                    continue;
                }
                //if (command == "test") { // pseudo block for tes modules
                //    ////_SB_PCI0B0D4PMAX PTDP PMIN TMAX PWRU
                //    //nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PTDP"));
                //    //res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PTDP"), &nType);
                //    //if (res) {
                //    //    cout << "PTDP = " << res->Argument[0].Argument << endl;
                //    //}
                //    //nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PMAX"));
                //    //res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PMAX"), &nType);
                //    //if (res) {
                //    //    cout << "PMAX = " << res->Argument[0].Argument << endl;
                //    //}
                //    //nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PMIN"));
                //    //res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PMIN"), &nType);
                //    //if (res) {
                //    //    cout << "PMIN = " << res->Argument[0].Argument << endl;
                //    //}
                //    //nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4TMAX"));
                //    //res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4TMAX"), &nType);
                //    //if (res) {
                //    //    cout << "TMAX = " << res->Argument[0].Argument << endl;
                //    //}
                //    //nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PWRU"));
                //    //res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PWRU"), &nType);
                //    //if (res) {
                //    //    cout << "PWRU = " << res->Argument[0].Argument << endl;
                //    //}
                //    int tRes = RunMainCommand(0x6, 0x0);
                //    continue;
                //}
                cout << "Unknown command - " << command << ", use \"usage\" or \"help\" for information" << endl;
            }
        } else {
            cout << "No supported hardware detected." << endl;
        }
    } else {
        cout << "Can't load driver or access denied. Are you in Test mode and admin?" << endl;
    }

    cout << "Done!" << endl;
}
