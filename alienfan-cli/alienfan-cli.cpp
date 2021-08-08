// alienfan-tools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <ole2.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <comdef.h>
#include "acpilib.h"
#include "acpi.h"

using namespace std;

#pragma comment(lib, "wbemuuid.lib")

_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));

struct ALIENFAN_TEMP {
    TCHAR* tempSensor;
    string tempSensorName;
};

struct ALIENFAN_FAN {
    TCHAR* fanBoostValue;
    string fanName;
    //TCHAR* fanSet;
    USHORT fanID;
};

struct ALIENFAN_MODEL {
    PWSTR modelName;
    TCHAR* unlockCommand;
    USHORT lockOn, lockOff;
    vector<ALIENFAN_TEMP> tempSensors;
    vector<ALIENFAN_FAN> fans;
    TCHAR* boostCommand;
    TCHAR* getRPMCommand;
    TCHAR* setRPMCommand;
};

ALIENFAN_MODEL* LoadModelsDB(PWSTR model) {
    ALIENFAN_MODEL* res = NULL;
    if (wstring(model) == L"Alienware m15") {
        res = new ALIENFAN_MODEL;
        res->modelName = model;
        res->unlockCommand = (TCHAR*) TEXT("\\____SB_IETMMCHG");
        res->boostCommand = (TCHAR*) TEXT("\\____SB_AMW1SRPM");
        res->getRPMCommand = (TCHAR*) TEXT("\\____SB_AMW1FNSR");
        res->setRPMCommand = NULL;
        res->lockOff = 0;
        res->lockOn = 1;
        
        ALIENFAN_FAN cFan;
        cFan.fanName = "CPU Fan";
        cFan.fanID = 0x32;
        cFan.fanBoostValue = (TCHAR*) TEXT("\\____SB_AMW1RPM1");
        res->fans.push_back(cFan);
        cFan.fanName = "GPU Fan";
        cFan.fanID = 0x33;
        cFan.fanBoostValue = (TCHAR*) TEXT("\\____SB_AMW1RPM2");
        res->fans.push_back(cFan);

        ALIENFAN_TEMP cTemp;
        cTemp.tempSensor = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_PCPT");
        cTemp.tempSensorName = "CPU Internal Thermistor";
        res->tempSensors.push_back(cTemp);
        cTemp.tempSensor = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_CUPT");
        cTemp.tempSensorName = "CPU External Thermistor";
        res->tempSensors.push_back(cTemp);
        cTemp.tempSensor = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH1R");
        cTemp.tempSensorName = "Graphic Internal Thermistor";
        res->tempSensors.push_back(cTemp);
        cTemp.tempSensor = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_GRAP");
        cTemp.tempSensorName = "Graphic External Thermistor";
        res->tempSensors.push_back(cTemp);
        cTemp.tempSensor = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH0R");
        cTemp.tempSensorName = "SSD Thermistor";
        res->tempSensors.push_back(cTemp);
        cTemp.tempSensor = (TCHAR*) TEXT("\\____SB_PCI0LPCBEC0_TH0L");
        cTemp.tempSensorName = "Ambient Thermistor";
        res->tempSensors.push_back(cTemp);
    }
    return res;
}

_bstr_t GetProperty(IWbemClassObject *pobj, PCWSTR pszProperty)
{
    _variant_t var;
    pobj->Get(pszProperty, 0, &var, nullptr, nullptr);
    return (_bstr_t) var;
}
void PrintProperty(IWbemClassObject *pobj, PCWSTR pszProperty)
{
    printf("%ls = %ls\n", pszProperty,
           static_cast<PWSTR>(GetProperty(pobj, pszProperty)));
}

PWSTR GetSystemInfo() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IWbemLocatorPtr spLocator;
    CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_ALL,
                     IID_PPV_ARGS(&spLocator));
    IWbemServicesPtr spServices;
    spLocator->ConnectServer(_bstr_t(L"root\\cimv2"),
                             nullptr, nullptr, 0, 0, nullptr, nullptr, &spServices);
    CoSetProxyBlanket(spServices, RPC_C_AUTHN_DEFAULT,
                      RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL,
                      RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
                      0, EOAC_NONE);
    IEnumWbemClassObjectPtr spEnum;
    spServices->ExecQuery(_bstr_t(L"WQL"),
                          _bstr_t(L"select * from Win32_ComputerSystem"),
                          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                          nullptr, &spEnum);
    IWbemClassObjectPtr spObject;
    ULONG cActual;
    while (spEnum->Next(WBEM_INFINITE, 1, &spObject, &cActual)
           == WBEM_S_NO_ERROR) {
        //PrintProperty(spObject, L"Name");
        //PrintProperty(spObject, L"Manufacturer");
        //PrintProperty(spObject, L"Model");
        return static_cast<PWSTR>(GetProperty(spObject, L"Model"));
    }
    return NULL;
}

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

void ProbeTemp() {
    PACPI_EVAL_OUTPUT_BUFFER resName = NULL;
    SHORT nsType = 0;
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
                cout << "Sensor #" << i << ", Name: " << senName << endl;
            }
        }
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
    std::cout << "AlienFan-cli v0.0.3.0\n";

    PWSTR model = GetSystemInfo();
    printf("System: %ls", model);

    ALIENFAN_MODEL* info = LoadModelsDB(model);

    if (info) {
        cout << " - Supported!" << endl;
    } else {
        cout << " - Unsupported!" << endl;
    }

    HANDLE acc = OpenAcpiService();

    if (argc < 2) 
    {
        Usage();
        return 1;
    }

    if (acc && QueryAcpiNSInLib()) {
        PACPI_EVAL_OUTPUT_BUFFER res;
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
                if (info && info->getRPMCommand) {
                    for (int i = 0; i < info->fans.size(); i++) {
                        res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(info->getRPMCommand, (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, info->fans[i].fanID));
                        if (res)
                            cout << "Fan " << info->fans[i].fanName << " is at " << res->Argument[0].Argument << " RPM." << endl;
                    }
                }
                continue;
            }
            if (command == "temp") {
                if (info) {
                    for (int i = 0; i < info->tempSensors.size(); i++) {
                        nType = GetNSType(info->tempSensors[i].tempSensor);
                        res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(info->tempSensors[i].tempSensor, &nType);
                        if (res)
                            cout << info->tempSensors[i].tempSensorName << ": " << res->Argument[0].Argument << endl;
                    }
                }
                continue;
            }
            if (command == "dump") {
                ACPI_NS_DATA data = {0};
                QueryAcpiNS(acc, &data, 0xc1);
                DumpAcpi(data);
                continue;
            }
            if (command == "probe") {
                ProbeTemp();
                continue;
            }
            if (command == "unlock") {
                if (info && info->unlockCommand) {
                    int lock = atoi(args[0].c_str());
                    if (lock) {
                        cout << "UnLocking device...";
                        //lock = info->lockOff;
                    }
                    else {
                        cout << "Locking device...";
                        //lock = info->lockOn;
                    }
                    res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(info->unlockCommand, (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, lock));
                    cout << " done." << endl;
                }
                continue;
            }
            if (command == "boost") {
                if (args.size() < 2) {
                    cout << "Boost: incorrect arguments" << endl;
                    continue;
                }
                if (info && info->boostCommand) {
                    ACPI_EVAL_INPUT_BUFFER_COMPLEX* acpiargs;
                    for (int i = 0; i < info->fans.size(); i++) {
                        UINT boost = atoi(args[i].c_str());
                        res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(info->fans[i].fanBoostValue, &nType);
                        if (res) {
                            cout << "Old boost:" << (UINT) res->Argument[0].Argument;
                        }
                        acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, info->fans[i].fanID);
                        acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(acpiargs, boost);
                        res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(info->boostCommand, acpiargs);
                        if (res) {
                            cout << ". Boost for fan " << info->fans[i].fanName << " done with code " << hex << res->Argument[0].Argument << dec;
                        }
                        res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(info->fans[i].fanBoostValue, &nType);
                        if (res) {
                            cout << ", New boost:" << (UINT) res->Argument[0].Argument << endl;
                        }
                    }
                }
                continue;
            }
            if (command == "test") { // pseudo block for tes modules
                //_SB_PCI0B0D4PMAX PTDP PMIN TMAX PWRU
                nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PTDP"));
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PTDP"), &nType);
                if (res) {
                    cout << "PTDP = " << res->Argument[0].Argument << endl;
                }
                nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PMAX"));
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PMAX"), &nType);
                if (res) {
                    cout << "PMAX = " << res->Argument[0].Argument << endl;
                }
                nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PMIN"));
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PMIN"), &nType);
                if (res) {
                    cout << "PMIN = " << res->Argument[0].Argument << endl;
                }
                nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4TMAX"));
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4TMAX"), &nType);
                if (res) {
                    cout << "TMAX = " << res->Argument[0].Argument << endl;
                }
                nType = GetNSType((TCHAR*) TEXT("\\____SB_PCI0B0D4PWRU"));
                res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue((TCHAR*) TEXT("\\____SB_PCI0B0D4PWRU"), &nType);
                if (res) {
                    cout << "PWRU = " << res->Argument[0].Argument << endl;
                }
                continue;
            }
            cout << "Unknown command - " << command << ", use \"usage\" or \"help\" for information" << endl;
        }
    } else {
        cout << "Can't load driver or access denied. Are you in Test mode and admin?" << endl;
    }

    cout << "Done!" << endl;
}
