#pragma once
#include <windowsx.h>
#include "ConfigHelper.h"
#include "alienfan-SDK.h"

class MonHelper {
private:
	HANDLE dwHandle;
public:
	ConfigHelper* conf;
	HWND dlg = NULL, fDlg = NULL;
	HANDLE stopEvent;
	AlienFan_SDK::Control* acpi;

	MonHelper(HWND, HWND, ConfigHelper*, AlienFan_SDK::Control*);
	~MonHelper();
	void Start();
	void Stop();
};

