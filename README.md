# alienfan-tools
Utilities for fan and power control for Alienware notebooks trough direct calls into ACPI BIOS.  
This is the side step from my [alienfx-tools](https://github.com/T-Troll/alienfx-tools) project, dedicated to fan and power control subsystem, and will be integrated into it later on.

Tools avaliable:
- `alienfan-cli` - simple fan control command line utility

## Disclamer
- This tools utilize low-level ACPI functions access, this can be dangerous and can provide system hang, BSOD and even hardware damage! Use with care, at you own risk!

## Known issues
- **DO NOT** try to run this tools at non-Alienware gear! It can provide dangerous result!
- ACPI driver can hang into stopping state. Run `alienfan-cli` without parameters a couple of times to reset it.
- Fan PRMs boost is quite fast, but if you lower boost, it took some time to slow down fans. 
- Power control only avaliable on some models, f.e. m15R1, m15R3.
- Power control modes not detected in power grow order, so check real PL1 after set using other tool, f.e. HWINFO.  
For my gear 0 = 60W unlocked, 1 - 60W, 2 - 75W, 3 - 45W with gpu boost, 4 - 45W. You gear can have other settings.
- Set Power control to non-zero value can block (lock back) fan control.
- As usual, AWCC service can interfere (reset values from time to time), so it's reccomended to stop it.

## Requrements
- Windows 10 x64 OS revision 1706 or higher. Any other OS **Does not supported!**
- "Test mode" should be enabled or integrity disabled. Issue `bcdedit /set testsigning on` (if you like warning sign at the desktop) or `bcdedit /set nointegritychecks on` command from Administarator command prompt to enable it, then reboot.
- Supported Alienware hardware.

## Installation
Unpack tools into folder, run exe.  
NB: You should have acpilib.dll and hwacc.sys into the same folder.

## Supported hardware
- Notebooks: `Alienware m15/17R1` or later.
- Desktops: `Alienware Aurora R7` or later (with issues, need more testing).

## `alienfan-cli` usage
it's a simple CLI fan control utility for now, providing the same functionality as AWCC (well... a bit more already).
ACPI can't control fans directly (well... i'm working on it), so all you can do is set fan boost (More RPM).
Run `alienfan-cli [command[=value{,value}] [command...]]`. After start, it detect you gear and show number of fans, temperature sensors and power modes avaliable.
Avaliable commands:
- `usage`, `help` - Show short help
- `dump` - Dump all ACPI values avaliable (for debug and new hardware support)
- `rpm` - Show current fan RPMs
- `temp` - Show known temperature sensors name and value
- `unlock` - Enable manual fan control
- `power=<value>` - Set PL1 to this predefined level. Possible levels autodetected from ACPI, see message at app start 
- `getfans` - Show current fan RPMs boost
- `setfans=<fan1>,<fan2>...` - Set fans RPM boost level (0..100 - in percent). Fan1 is for CPU fan, Fan2 for GPU one. Number of arguments should be the same as number of fans application detect

## ToDo:
- [x] Temperature sensors reading
- [ ] Eliminate "Test mode" requirement
- [x] Additional hardware support (thanks for ACPI dumps, provided by `alienfx-tools` community!):
- [x] SDK lib for easy sharing
- [ ] GUI 
- [ ] Temp-RPM curves and indirect RPM control
- [x] CPU power limit control
- [ ] Dynamic power distribution
- [ ] `alienfx-gui` integration

## Tools Used
* Visual Studio Community 2019

## License
MIT. You can use these tools for any non-commercial or commercial use, modify it any way - supposing you provide a link to this page from you product page and mention me as the one of authors.

## Credits
Idea, code and hardware support by T-Troll.  
ACPI SDK and driver based on kdshk's [WindowsHwAccess](https://github.com/kdshk/WindowsHwAccess).  
Special thanks to [DavidLapous](https://github.com/DavidLapous) for inspiration and advices!


