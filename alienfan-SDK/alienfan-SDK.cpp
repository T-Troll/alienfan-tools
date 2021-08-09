// alienfan-SDK.cpp : Defines the functions for the static library.
//

#include "alienfan-SDK.h"
#include "acpilib.h"

namespace AlienFan_SDK {

	// One string to rule them all!
	// AMW interface command - 3 parameters (not used, command, buffer).
	TCHAR* mainCommand = (TCHAR*) TEXT("\\____SB_AMW1WMAX");

	Control::Control() {
		activated = (acc = OpenAcpiService()) && QueryAcpiNSInLib();
	}
	Control::~Control() {
		sensors.clear();
	}
	int Control::RunMainCommand(short command, byte subcommand, byte value1, byte value2) {
		if (activated) {
			PACPI_EVAL_OUTPUT_BUFFER res = NULL;
			ACPI_EVAL_INPUT_BUFFER_COMPLEX* acpiargs;
			BYTE operand[4] = {subcommand, value1, value2, 0};
			//UINT operand = ((UINT) arg2) << 16 | (UINT) arg1 << 8 | subcommand;
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(NULL, 0);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutIntArg(acpiargs, command);
			acpiargs = (ACPI_EVAL_INPUT_BUFFER_COMPLEX*) PutBuffArg(acpiargs, 4, operand);
			res = (ACPI_EVAL_OUTPUT_BUFFER*) EvalAcpiNSArgOutput(mainCommand, acpiargs);
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
				sensors.clear();
				fans.clear();
				powers.clear();
				// check how many fans we have...
				USHORT fanID = 0x32;
				while (RunMainCommand(0x13, 1, (BYTE) fanID) == 1) {
					fans.push_back(fanID);
					fanID++;
				}
				// check how many power states...
				BYTE startIndex = 4, powerID = 0;
				powers.push_back(0);
				while ((powerID = RunMainCommand(0x14, 0x3, startIndex)) != 0xff) {
					powers.push_back(powerID);
					startIndex++;
				}
				ALIENFAN_SEN_INFO cur;
				cur.senIndex = 1; // TODO - get real ID from AWC!
				cur.isFromAWC = true;
				cur.name = "CPU Internal Thermister";
				sensors.push_back(cur);
				cur.senIndex = 6; // TODO - get real ID from AWC!
				cur.name = "GPU Internal Thermister";
				sensors.push_back(cur);
				for (int i = 0; i < 10; i++) {
					tempNamePattern[23] = i + '0';
					nsType = GetNSType(tempNamePattern);
					if (nsType != -1) {
						// Key found!
						if (resName = (PACPI_EVAL_OUTPUT_BUFFER) GetNSValue(tempNamePattern, (USHORT*)&nsType)) {
							char* c_name = new char[resName->Argument[0].DataLength + 1];
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
			return RunMainCommand(0x14, 0x5, (byte)fans[fanID]);
		return -1;
	}
	int Control::GetFanValue(int fanID) {
		if (fanID < fans.size())
			return RunMainCommand(0x14, 0xc, (byte)fans[fanID]);
		return -1;
	}
	bool Control::SetFanValue(int fanID, byte value) {
		if (fanID < fans.size())
			return RunMainCommand(0x15, 0x2, (byte)fans[fanID], value) != (-1);
		return false;
	}
	int Control::GetTempValue(int TempID) {
		// Additional temp sensor value pattern
		TCHAR tempValuePattern[] = TEXT("\\____SB_PCI0LPCBEC0_SEN1_TMP");
		if (TempID < sensors.size()) {
			if (sensors[TempID].isFromAWC)
				return RunMainCommand(0x14, 0x4, (byte) sensors[TempID].senIndex);
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
		return RunMainCommand(0x15, 0x1);
	}
	int Control::SetPower(int level) {
		//BYTE unlockType = 0;
		//switch (level) {
		//case 1: // 45W GPU boost
		//	unlockType = 0xa3; break;
		//case 2: // 45W calm
		//	unlockType = 0xa2; break;
		//case 3: // 60W
		//	unlockType = 0xa0; break;
		//case 4: // 75W
		//	unlockType = 0xa1; break;
		//}
		if (level < powers.size())
			return RunMainCommand(0x15, 0x1, (byte)powers[level]);
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
}
