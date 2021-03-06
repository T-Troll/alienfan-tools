#pragma once
#include <windowsx.h>
#include "ConfigHelper.h"
#include "alienfan-SDK.h"

class MonHelper {
private:
	HANDLE dwHandle = 0;
public:
	ConfigHelper* conf;
	HWND dlg = NULL, fDlg = NULL;
	HANDLE stopEvent = 0;
	short oldPower = 0;
	AlienFan_SDK::Control* acpi;
	vector<int> senValues, fanValues, boostValues, boostSets;

	MonHelper(HWND, HWND, ConfigHelper*, AlienFan_SDK::Control*);
	~MonHelper();
	void Start();
	void Stop();
};

