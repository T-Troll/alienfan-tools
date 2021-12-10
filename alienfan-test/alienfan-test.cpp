// alienfan-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "alienfan-SDK.h"
using namespace std;



int main()
{

    printf("Opening ACPI...\n");

    AlienFan_SDK::Control *acpi = new AlienFan_SDK::Control();

    if (acpi->IsActivated()) {
        printf("Probing...\n");
        acpi->Probe();
    }

    delete acpi;

    printf("Goodbye!\n");
}
