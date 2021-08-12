#pragma once

#include <windows.h>
#include <vector>

using namespace std;

struct fan_point {
	short temp = 0;
	short boost = 0;
};

struct fan_block {
	short fanIndex = 0;
	vector<fan_point> points;
};

struct temp_block {
	short sensorIndex = 0;
	vector<fan_block> fans;
};

class ConfigHelper {
private:
	HKEY   hKey1, hKey2;
public:
	DWORD lastPowerStage = 0;
	DWORD lastSelectedFan = 0;
	DWORD lastSelectedSensor = 0;
	vector<temp_block> tempControls;

	NOTIFYICONDATA niData = {0};

	ConfigHelper();
	~ConfigHelper();
	temp_block* FindSensor(int);
	fan_block* FindFanBlock(temp_block*, int);
	void Load();
	void Save();
};

