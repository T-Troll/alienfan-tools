#pragma once

#include <windows.h>
#include <string>
#include <vector>

using namespace std;

namespace AlienFan_SDK {

	struct ALIENFAN_SEN_INFO {
		SHORT senIndex = 0;
		string name;
		bool isFromAWC = false;
	};

	class Control {
	private:
		HANDLE acc = NULL;
		bool activated = false;
		//int RunMainCommand(short command, byte subcommand, byte value1 = 0, byte value2 = 0);
	public:
		Control();
		~Control();
		bool Probe();
		int GetFanRPM(int fanID);
		int GetFanValue(int fanID);
		bool SetFanValue(int fanID, byte value);
		int GetTempValue(int TempID);
		int Unlock();
		int SetPower(int level);
		HANDLE GetHandle();
		bool IsActivated();
		int HowManyFans();
		int HowManyPower();
		int HowManySensors();

		int RunMainCommand(short command, byte subcommand, byte value1 = 0, byte value2 = 0);

		vector<ALIENFAN_SEN_INFO> sensors;
		vector<USHORT> fans;
		vector<USHORT> powers;
	};
}
