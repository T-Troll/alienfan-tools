#include "MonHelper.h"
#include <CommCtrl.h>
#include "resource.h"

DWORD WINAPI CMonProc(LPVOID);

MonHelper::MonHelper(HWND cDlg, ConfigHelper* config, AlienFan_SDK::Control* acp) {
	dlg = cDlg;
	conf = config;
	acpi = acp;
	stopEvent = CreateEvent(NULL, false, false, NULL);
	Start();
}

MonHelper::~MonHelper() {
	Stop();
	CloseHandle(stopEvent);
}

void MonHelper::Start() {
		// start thread...
	if (!dwHandle) {
#ifdef _DEBUG
		OutputDebugString("Mon thread start.\n");
#endif
		ResetEvent(stopEvent);
		dwHandle = CreateThread(NULL, 0, CMonProc, this, 0, NULL);
	}
}

void MonHelper::Stop() {
	if (dwHandle) {
#ifdef _DEBUG
		OutputDebugString("Mon thread stop.\n");
#endif
		SetEvent(stopEvent);
		WaitForSingleObject(dwHandle, 1000);
		CloseHandle(dwHandle);
		dwHandle = 0;
	}
}

DWORD WINAPI CMonProc(LPVOID param) {
	MonHelper* src = (MonHelper*) param;
	vector<int> senValues, fanValues, boostValues, boostSets;
	senValues.resize(src->acpi->HowManySensors());
	fanValues.resize(src->acpi->HowManyFans());
	boostValues.resize(src->acpi->HowManyFans());
	boostSets.resize(src->acpi->HowManyFans());

	HWND tempList = GetDlgItem(src->dlg, IDC_TEMP_LIST),
		rpmState = GetDlgItem(src->dlg, IDC_FAN_RPM),
		curveBlock = GetDlgItem(src->dlg, IDC_FAN_CURVE);

	while (WaitForSingleObject(src->stopEvent, 500) == WAIT_TIMEOUT) {
		// update values.....

		// temps..
		for (int i = 0; i < src->acpi->HowManySensors(); i++) {
			int sValue = src->acpi->GetTempValue(i);
			if (sValue != senValues[i]) {
				senValues[i] = sValue;
				string name = to_string(sValue);
				ListView_SetItemText(tempList, i, 0, (LPSTR) name.c_str());
			}
		}

		// fans...
		if (src->conf->lastSelectedFan != -1) {
			for (int i = 0; i < src->acpi->HowManyFans(); i++) {
				int rpValue = src->acpi->GetFanRPM(i);
				boostValues[i] = src->acpi->GetFanValue(i);
				boostSets[i] = 0;
				if (rpValue != fanValues[i]) {
					// Update RPM block...
					if (i == src->conf->lastSelectedFan) {
						string rpmText = "Current RPM: " + to_string(rpValue);
						Static_SetText(rpmState, rpmText.c_str());
					}
				}
			}
		}

		// boosts..
		if (src->conf->lastPowerStage == 0) {
			// in manual mode, can set...
			for (int i = 0; i < src->conf->tempControls.size(); i++) {
				temp_block* sen = &src->conf->tempControls[i];
				for (int j = 0; j < sen->fans.size(); j++) {
					fan_block* fan = &sen->fans[j];
					// Look for boost point for temp...
					for (int k = 1; k < fan->points.size(); k++) {
						if (senValues[sen->sensorIndex] <= fan->points[k].temp) {
							int tBoost = fan->points[k - 1].boost +
								(fan->points[k].boost - fan->points[k - 1].boost) *
								(senValues[sen->sensorIndex] - fan->points[k - 1].temp) /
								(fan->points[k].temp - fan->points[k - 1].temp);
							if (tBoost > boostSets[fan->fanIndex])
								boostSets[fan->fanIndex] = tBoost;
							break;
						}
					}
				}
			}
			// Now set if needed...
			for (int i = 0; i < src->acpi->HowManyFans(); i++)
				if (boostSets[i] != boostValues[i]) {
					src->acpi->SetFanValue(i, boostSets[i]);
					if (i == src->conf->lastSelectedFan)
						SendMessage(curveBlock, WM_PAINT, 0, 0);
				}
		}

	}
	return 0;
}
