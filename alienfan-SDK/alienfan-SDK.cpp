// alienfan-SDK.cpp : Defines the functions for the static library.
//

#include "alienfan-SDK.h"
#include "acpilib.h"

namespace AlienFan_SDK {

	// One string to rule them all!
	// AMW interface com - 3 parameters (not used, com, buffer).
	TCHAR* mainCommand = (TCHAR*) TEXT("\\____SB_AMW1WMAX");

	// GPU control interface
	TCHAR* gpuCommand = (TCHAR*) TEXT("\\____SB_PCI0PEG0PEGPGPS_");
	// arg0 not used, arg1 always 0x100, arg2 is command, arg3 is DWORD mask.

	Control::Control() {

		acc = OpenAcpiService();
		switch ((LONG_PTR)acc) {
		case -1:
			return;
		case 0:
			wrongEnvironment = true;
			return;
		default:
			activated = QueryAcpiNSInLib();
		}
	}
	Control::~Control() {
		sensors.clear();
		CloseDll();
	}

	void Control::UnloadService() {
		if (acc != INVALID_HANDLE_VALUE && acc)
			CloseAcpiService(acc);
		acc = NULL;
	}

	int Control::RunMainCommand(short com, byte sub, byte value1, byte value2) {
		if (activated && com) {
			PACPI_EVAL_OUTPUT_BUFFER res = NULL;
			ACPI_EVAL_INPUT_BUFFER_COMPLEX* acpiargs;
			BYTE operand[4] = {sub, value1, value2, 0};
			//UINT operand = ((UINT) arg2) << 16 | (UINT) arg1 << 8 | sub;
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(acpiargs, com);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutBuffArg(acpiargs, 4, operand);
			res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(mainCommand, acpiargs);
			if (res) {
				return (int) res->Argument[0].Argument;
			}
		}
		return -1;
	}

	int Control::RunGPUCommand(short com, DWORD packed) {
		if (activated && com) {
			PACPI_EVAL_OUTPUT_BUFFER res = NULL;
			ACPI_EVAL_INPUT_BUFFER_COMPLEX* acpiargs;
			
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(acpiargs, 0x100);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(acpiargs, com);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutBuffArg(acpiargs, 4, (UCHAR*)&packed);
			res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(gpuCommand, acpiargs);
			if (res) {
				return (int) res->Argument[0].Argument;
			}
		}
		return -1;
	}

	bool Control::Probe() {
		// Additional temp sensor name pattern
		TCHAR tempNamePattern[] = TEXT("\\____SB_PCI0LPCBEC0_SEN1_STR");
		if (activated) {
			PACPI_EVAL_OUTPUT_BUFFER resName = NULL;
			USHORT nsType = GetNSType(mainCommand);
			if (nsType == ACPI_TYPE_METHOD) {
				// Alienware device detected!
				// ToDo: Now check device type
				aDev = 0;
				sensors.clear();
				fans.clear();
				powers.clear();
				// check how many fans we have...
				USHORT fanID = 0x32;
				while (RunMainCommand(devs[aDev].getFanID.com, devs[aDev].getFanID.sub, (BYTE) fanID) == 1) {
					fans.push_back(fanID);
					fanID++;
				}
				// check how many power states...
				BYTE startIndex = 4, powerID = 0;
				powers.push_back(0);
				while ((powerID = RunMainCommand(devs[aDev].getPowerID.com, devs[aDev].getPowerID.sub, startIndex)) != 0xff) {
					powers.push_back(powerID);
					startIndex++;
				}
				ALIENFAN_SEN_INFO cur;
				// Scan term sensors for fans...
				for (int i = 0; i < fans.size(); i++) {
					USHORT tempIndex;
					if ((tempIndex = RunMainCommand(devs[aDev].getZoneSensorID.com, devs[aDev].getZoneSensorID.sub, (byte)fans[i])) != -1) {
						cur.senIndex = tempIndex;
						cur.isFromAWC = true;
						switch (i) {
						case 0:
							cur.name = "CPU Internal Thermistor"; break;
						case 1:
							cur.name = "GPU Internal Thermistor"; break;
						default:
							cur.name = "FAN#" + to_string(i) + " sensor"; break;
						}
						sensors.push_back(cur);
					}
				}
				for (int i = 0; i < 10; i++) {
					tempNamePattern[23] = i + '0';
					nsType = GetNSType(tempNamePattern);
					if (nsType != -1) {
						// Key found!
						if (resName = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(tempNamePattern, (USHORT*)&nsType)) {
							char* c_name = new char[1 + resName->Argument[0].DataLength];
							wcstombs_s(NULL, c_name, resName->Argument[0].DataLength, (TCHAR*) resName->Argument[0].Data, resName->Argument[0].DataLength);
							string senName = c_name;
							delete[] c_name;
							cur.senIndex = i;
							cur.name = senName;
							cur.isFromAWC = false;
							sensors.push_back(cur);
							//cout << "Sensor #" << i << ", Name: " << senName << endl;
						}
					}
				}
				return true;
			}
		}
		return false;
	}
	int Control::GetFanRPM(int fanID) {
		if (fanID < fans.size())
			return RunMainCommand(devs[aDev].getFanRPM.com, devs[aDev].getFanRPM.sub, (byte)fans[fanID]);
		return -1;
	}
	int Control::GetFanValue(int fanID) {
		//TCHAR fanValueCommand[] = TEXT("\\____SB_AMW1RPM1");
		//PACPI_EVAL_OUTPUT_BUFFER res = NULL;
		if (fanID < fans.size()) {
			//fanValueCommand[15] = '1' + fanID;
			//USHORT nsType = GetNSType(fanValueCommand);
			//if (nsType) {
			//	res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(fanValueCommand, &nsType);
			//	if (res)
			//		return res->Argument[0].Argument;
			//} else
			return RunMainCommand(devs[aDev].getFanBoost.com, devs[aDev].getFanBoost.sub, (byte) fans[fanID]);
		}
		return -1;
	}
	bool Control::SetFanValue(int fanID, byte value) {
		if (fanID < fans.size())
			return RunMainCommand(devs[aDev].setFanBoost.com, devs[aDev].setFanBoost.sub, (byte)fans[fanID], value) != (-1);
		return false;
	}
	int Control::GetTempValue(int TempID) {
		// Additional temp sensor value pattern
		TCHAR tempValuePattern[] = TEXT("\\____SB_PCI0LPCBEC0_SEN1_TMP");
		if (TempID < sensors.size()) {
			if (sensors[TempID].isFromAWC)
				return RunMainCommand(devs[aDev].getTemp.com, devs[aDev].getTemp.sub, (byte) sensors[TempID].senIndex);
			else {
				PACPI_EVAL_OUTPUT_BUFFER res = NULL;
				tempValuePattern[23] = sensors[TempID].senIndex + '0';
				USHORT nsType = GetNSType(tempValuePattern);
				res = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(tempValuePattern, &nsType);
				if (res)
					return (res->Argument[0].Argument - 0xaac) / 0xa;
			}
		}
		return -1;
	}
	int Control::Unlock() {
		return RunMainCommand(devs[aDev].setPower.com, devs[aDev].setPower.sub);
	}
	int Control::SetPower(int level) {
		if (level < powers.size())
			return RunMainCommand(devs[aDev].setPower.com, devs[aDev].setPower.sub, (byte)powers[level]);
		return -1;
	}
	int Control::GetPower() {
		int pl = RunMainCommand(devs[aDev].getPower.com, devs[aDev].getPower.sub);
		for (int i = 0; pl > 0 && i < powers.size(); i++)
			if (powers[i] == pl)
				return i;
		return -1;
	}
	int Control::SetGPU(int power) {
		if (power >= 0 && power < 5) {
			return RunGPUCommand(devs[aDev].setGPUPower.com, power << 4 | devs[aDev].setGPUPower.sub);
		}
		return -1;
	}
	HANDLE Control::GetHandle() {
		return acc;
	}
	bool Control::IsActivated() {
		return activated;
	}
	int Control::HowManyFans() {
		return (int)fans.size();
	}
	int Control::HowManyPower() {
		return (int)powers.size();
	}
	int Control::HowManySensors() {
		return (int)sensors.size();
	}
}
