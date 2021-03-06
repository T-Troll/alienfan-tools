
#define WIN32_LEAN_AND_MEAN
//#include <iostream>
#include <stdio.h>
#include <vector>
#include <combaseapi.h>
#include <PowrProf.h>
#include "alienfan-SDK.h"
#include "alienfan-low.h"

#pragma comment(lib, "PowrProf.lib")

using namespace std;

bool CheckArgs(string cName, int minArgs, size_t nargs) {
    if (minArgs < nargs) {
        printf("%s: Incorrect arguments (should be %d)\n", cName.c_str(), minArgs);
        return false;
    }
    return true;
}

void Usage() {
    printf("Usage: alienfan-cli [command[=value{,value}] [command...]]\n\
Avaliable commands: \n\
usage, help\t\t\tShow this usage\n\
rpm[=id]\t\t\tShow fan(s) RPM\n\
persent[=id]\t\t\tShow fan(s) RPM in perecent of maximum\n\
temp[=id]\t\t\tShow known temperature sensors values\n\
unlock\t\t\t\tUnclock fan controls\n\
getpower\t\t\tDisplay current power state\n\
setpower=<mode>\t\t\tSet CPU power to this mode\n\
setgpu=<value>\t\t\tSet GPU power limit\n\
setperf=<ac>,<dc>\t\tSet CPU performance boost\n\
getfans[=<mode>]\t\tShow current fan boost level (0..100 - in percent) with selected mode\n\
setfans=<fan1>[,<fan2>[,mode]]\tSet fans boost level (0..100 - in percent) with selected mode\n\
resetcolor\t\t\tReset color system\n\
setcolor=<mask>,r,g,b\t\tSet light(s) defined by mask to color\n\
setcolormode=<dim>,<flag>\tSet light system brightness and mode\n\
direct=<id>,<subid>[,val,val]\tIssue direct interface command (for testing)\n\
directgpu=<id>,<value>\t\tIssue direct GPU interface command (for testing)\n\
\tPower mode can be in 0..N - according to power states detected\n\
\tPerformance boost can be in 0..4 - disabled, enabled, aggresive, efficient, efficient aggresive\n\
\tGPU power limit can be in 0..4 - 0 - no limit, 4 - max. limit\n\
\tNumber of fan boost values should be the same as a number of fans detected\n\
\tMode can be 0 or absent for set cooked value, 1 for raw value\n\
\tBrighness for ACPI lights can only have 10 values - 1,3,4,6,7,9,10,12,13,15\n\
\tAll values in \"direct\" commands should be hex, not decimal!\n");
}

int main(int argc, char* argv[])
{
    printf("AlienFan-cli v5.5.0\n");

    AlienFan_SDK::Control *acpi = new AlienFan_SDK::Control();

    bool supported = false;

    if (acpi->IsActivated()) {

        AlienFan_SDK::Lights *lights = new AlienFan_SDK::Lights(acpi);

        if (supported = acpi->Probe()) {
            printf("Supported hardware v%d detected, %d fans, %d sensors, %d power states. Light control %s.\n",
                   acpi->GetVersion(), (int) acpi->HowManyFans(), (int) acpi->sensors.size(), (int) acpi->HowManyPower(),
                   (lights->IsActivated() ? "enabled" : "disabled"));
        }
        else {
           printf("Supported hardware not found!\n");
        }

        if (argc < 2) 
        {
            Usage();
        } else {

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
                //cerr << "Executing " << command << " with " << values);
                if (command == "usage" || command == "help" || command == "?") {
                    Usage();
                    continue;
                }
                if (command == "rpm" && supported) {
                    int rpms = 0;
                    if (args.size() > 0 && atoi(args[0].c_str()) < acpi->HowManyFans()) {
                        printf("%d\n", acpi->GetFanRPM(atoi(args[0].c_str())));
                    } else {
                        for (int i = 0; i < acpi->HowManyFans(); i++)
                            if ((rpms = acpi->GetFanRPM(i)) >= 0)
                                printf("Fan#%d: %d\n", i, rpms);
                            else {
                                printf("RPM reading failed!\n");
                                break;
                            }
                    }
                    continue;
                }
                if (command == "percent" && supported) {
                    int prms = 0;
                    if (args.size() > 0 && atoi(args[0].c_str()) < acpi->HowManyFans()) {
                        printf("%d\n", acpi->GetFanPercent(atoi(args[0].c_str())));
                    } else {
                        for (int i = 0; i < acpi->HowManyFans(); i++)
                            if ((prms = acpi->GetFanPercent(i)) >= 0)
                                printf("Fan#%d: %d%%\n", i, prms);
                            else {
                                printf("RPM percent reading failed!\n");
                                break;
                            }
                    }
                    continue;
                }
                if (command == "temp" && supported) {
                    int res = 0;
                    if (args.size() > 0 && atoi(args[0].c_str()) < acpi->HowManySensors()) {
                        printf("%d\n", acpi->GetTempValue(atoi(args[0].c_str())));
                    } else {
                        for (int i = 0; i < acpi->HowManySensors(); i++) {
                            printf("%s: %d\n", acpi->sensors[i].name.c_str(), acpi->GetTempValue(i));
                        }
                    }
                    continue;
                }
                if (command == "unlock" && supported) {
                    if (acpi->Unlock() >= 0)
                        printf("Unlock successful.\n");
                    else
                        printf("Unlock failed!\n");
                    continue;
                }
                if (command == "setpower" && supported && CheckArgs(command, 1, args.size())) {

                    BYTE unlockStage = atoi(args[0].c_str());
                    if (unlockStage < acpi->HowManyPower()) {
                        printf("Power set to %d (result %d)\n", unlockStage, acpi->SetPower(unlockStage));
                    } else
                        printf("Power: incorrect value (should be 0..%d\n", acpi->HowManyPower());
                    continue;
                }
                if (command == "setgpu" && supported && CheckArgs(command, 1, args.size())) {

                    BYTE gpuStage = atoi(args[0].c_str());
                    if (acpi->SetGPU(gpuStage) >= 0)
                        printf("GPU limit set to %d", gpuStage);
                    else
                        printf("GPU limit set failed!\n");
                    continue;
                }
                if (command == "setperf" && CheckArgs(command, 2, args.size())) {
                    DWORD acMode = atoi(args[0].c_str()),
                        dcMode = atoi(args[1].c_str());
                    if (acMode > 4 || dcMode > 4)
                        printf("Incorrect value - should be 0..4\n");
                    else {
                        GUID* sch_guid, perfset;
                        IIDFromString(L"{be337238-0d82-4146-a960-4f3749d470c7}", &perfset);
                        PowerGetActiveScheme(NULL, &sch_guid);
                        PowerWriteACValueIndex(NULL, sch_guid, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &perfset, acMode);
                        PowerWriteDCValueIndex(NULL, sch_guid, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &perfset, dcMode);
                        PowerSetActiveScheme(NULL, sch_guid);
                        printf("CPU boost set to %d,%d\n", acMode, dcMode);
                        //PowerReadACValueIndex(NULL, sch_guid, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &perfset, &acMode);
                        //PowerReadDCValueIndex(NULL, sch_guid, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &perfset, &dcMode);
                        LocalFree(sch_guid);
                    }
                    continue;
                }
                if (command == "getpower" && supported) {
                    int cpower = acpi->GetPower();
                    if (cpower >= 0)
                        printf("Current power mode: %d\n", cpower);
                    else
                        printf("Getpower failed!\n");
                    continue;
                }
                if (command == "getfans" && supported) {
                    int prms;  bool direct = false;
                    if (args.size())
                        direct = atoi(args[0].c_str()) > 0;
                    for (int i = 0; i < acpi->HowManyFans(); i++)
                        if ((prms = acpi->GetFanValue(i, direct)) >= 0)
                            printf("Fan#%d boost %d\n", i, prms);
                        else {
                            printf("Get fan settings failed!\n");
                            break;
                        }
                    continue;
                }
                if (command == "setfans" && supported && CheckArgs(command, acpi->HowManyFans(), args.size())) {

                    bool direct = false;
                    if (args.size() > acpi->HowManyFans())
                        direct = atoi(args[acpi->HowManyFans()].c_str()) > 0;
                    for (int i = 0; i < acpi->HowManyFans(); i++) {
                        BYTE boost = atoi(args[i].c_str());
                        if (acpi->SetFanValue(i, boost, direct) >= 0)
                            printf("Fan#%d boost set to %d\n", i, boost);
                        else {
                            printf("Set fan level failed!\n");
                            break;
                        }
                    }
                    continue;
                }
                if (command == "direct" && CheckArgs(command, 2, args.size()) ) {
                    //int res = 0;

                    AlienFan_SDK::ALIENFAN_COMMAND comm;
                    comm.com = (byte) strtoul(args[0].c_str(), NULL, 16);
                    comm.sub = (byte) strtoul(args[1].c_str(), NULL, 16);
                    byte value1 = 0, value2 = 0;
                    if (args.size() > 2)
                        value1 = (byte) strtol(args[2].c_str(), NULL, 16);
                    if (args.size() > 3)
                        value2 = (byte) strtol(args[3].c_str(), NULL, 16);
                    //if ((res = acpi->RunMainCommand(comm, value1, value2)) != acpi->GetErrorCode())
                        printf("Direct call result: %d\n", acpi->RunMainCommand(comm, value1, value2));
                    //else {
                    //    printf("Direct call failed!\n");
                    //    break;
                    //}
                    continue;
                }
                if (command == "directgpu" && CheckArgs(command, 2, args.size())) {
                    //int res = 0;
                    USHORT command = (USHORT) strtoul(args[0].c_str(), NULL, 16);// atoi(args[0].c_str());
                    DWORD subcommand = strtoul(args[1].c_str(), NULL, 16);// atoi(args[1].c_str());
                    //if ((res = acpi->RunGPUCommand(command, subcommand)) != acpi->GetErrorCode())
                        printf("DirectGPU call result: %d\n", acpi->RunGPUCommand(command, subcommand));
                    //else {
                    //    printf("DirectGPU call failed!\n");
                    //    break;
                    //}
                    continue;
                }

                if (command == "resetcolor" && lights->IsActivated()) { // Reset color system for Aurora
                    if (lights->Reset())
                        printf("Lights reset complete\n");
                    else
                        printf("Lights reset failed\n");
                    continue;
                }
                if (command == "setcolor" && lights->IsActivated() && CheckArgs(command, 4, args.size())) { // Set light color for Aurora

                    byte mask = atoi(args[0].c_str()),
                        r = atoi(args[1].c_str()),
                        g = atoi(args[2].c_str()),
                        b = atoi(args[3].c_str());
                    if (lights->SetColor(mask, r, g, b))
                        printf("SetColor complete.\n");
                    else
                        printf("SetColor failed.\n");
                    lights->Update();
                    continue;
                }
                if (command == "setcolormode" && lights->IsActivated() && CheckArgs(command, 2, args.size())) { // set effect (?) for Aurora

                    byte num = atoi(args[0].c_str()),
                        mode = atoi(args[1].c_str());
                    if (lights->SetMode(num, mode)) {
                        printf("SetColorMode complete.\n");
                    } else
                        printf("SetColorMode failed.\n");
                    lights->Update();
                    continue;
                }
                //if (command == "test") { // pseudo block for test modules
                //    continue;
                //}
                printf("Unknown command - %s, use \"usage\" or \"help\" for information\n", command.c_str());
            }
        }

        delete lights;

    } else {
        printf("System configuration issue - see readme.md for details!\n");
    }

    delete acpi;

}
