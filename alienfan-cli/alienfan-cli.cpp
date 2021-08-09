// alienfan-tools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "alienfan-SDK.h"
#include "acpilib.h"
//#include "acpi.h"

using namespace std;

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
    std::cout << "AlienFan-cli v0.0.5\n";

    AlienFan_SDK::Control acpi;

    if (argc < 2) 
    {
        Usage();
        return 1;
    }

    if (acpi.IsActivated()) {

        if (acpi.Probe()) {
            cout << "Supported hardware detected, " << acpi.HowManyFans() << " fans, " 
                << acpi.sensors.size() << " sensors, " << acpi.HowManyPower() << " power states." << endl;

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
                    int prms = 0;
                    for (int i = 0; i < acpi.HowManyFans(); i++)
                        if ((prms = acpi.GetFanRPM(i)) >= 0)
                            cout << "Fan#" << i << ": " << prms << endl;
                        else {
                            cout << "RPM reading failed!" << endl;
                            break;
                        }
                    //prms = RunMainCommand(0x14, 0x8, 0x32);
                    //if (prms >= 0)
                    //    cout << "Target CPU fan: " << prms;
                    //prms = RunMainCommand(0x14, 0x8, 0x33);
                    //if (prms >= 0)
                    //    cout << ", Target GPU fan: " << prms << endl;
                    continue;
                }
                if (command == "temp") {
                    int res = 0;
                    for (int i = 0; i < acpi.sensors.size(); i++) {
                        if ((res = acpi.GetTempValue(i)) >= 0)
                            cout << acpi.sensors[i].name << ": " << res << endl;
                    }
                    continue;
                }
                if (command == "dump") {
                    ACPI_NS_DATA data = {0};
                    QueryAcpiNS(acpi.GetHandle(), &data, 0xc1);
                    DumpAcpi(data);
                    continue;
                }
                if (command == "unlock") {
                    if (acpi.Unlock() >= 0)
                        cout << "Unlock successful." << endl;
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
                    if (unlockStage < acpi.HowManyPower()) {
                        if (acpi.SetPower(unlockStage) >= 0)
                            cout << "Power set to " << (int) unlockStage << endl;
                        else
                            cout << "Power set failed!" << endl;
                    } else
                        cout << "Power: incorrect value (should be 0.." << acpi.HowManyPower() << ")" << endl;
                    continue;
                }
                if (command == "getfans") {
                    int prms = 0;
                    for (int i = 0; i < acpi.HowManyFans(); i++)
                        if ((prms = acpi.GetFanValue(i)) >= 0)
                            cout << "Fan#" << i << " now at " << prms << endl;
                        else {
                            cout << "Get fan settings failed!" << endl;
                            break;
                        }
                    continue;
                }
                if (command == "setfans") {
                    if (args.size() < acpi.HowManyFans()) {
                        cout << "Setfans: incorrect arguments (should be " << acpi.HowManyFans() << ")" << endl;
                        continue;
                    }
                    int prms = 0;
                    for (int i = 0; i < acpi.HowManyFans(); i++) {
                        BYTE boost = atoi(args[i].c_str());
                        if (acpi.SetFanValue(i, boost))
                            cout << "Fan#" << i << " set to " << (int) boost << endl;
                        else {
                            cout << "Set fan level failed!" << endl;
                            break;
                        }
                    }
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
            cout << "Supported hardware not found!" << endl;
        }
    } else {
        cout << "Driver initialization issue. Are you admin?" << endl;
    }
    //cout << "Done!" << endl;
}
