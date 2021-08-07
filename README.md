# alienfan-tools
Utilities for fan and power control for Alienware notebooks trough direct calls into ACPI BIOS.  
This is the side step from my [alienfx-tools](https://github.com/T-Troll/alienfx-tools) project, dedicated to fan and power control subsystem, and will be integrated into it later on.

Tools avaliable:
- `alienfan-cli` - simple fan control command line utility

## Disclamer
- This tools utilize low-level ACPI functions access, this can be dangerous for unsupported hardware and can provide system hangs, BSODs and even hardware damage! Use with care, for supported hardware only! Use it at you own risk!

## Known issues
- **DO NOT** try to run this tools for other gear! It can provide dangerous result!
- After unsuccessful set, ACPI driver can hang into stopping state. Run `alienfan-cli` without parameters a couple of times to reset it (you should see RPM reading if reset succesful).
- Boost PRMs increased quite fast, but if you lower boost, it took some time to slow down fans. 

## Requrements
- Windows 10 x64 OS revision 1706 or higher. Any other OS **Does not supported!**
- "Test mode" should be enabled. Issue `bcdedit /set testsigning on` command from Administarator command prompt to enable it, then reboot.
- Supported Alienware hardware.

## Installation
Unpack tools into folder, run any exe.  
NB: You should have acpilib.dll and hwacc.sys into the same folder.

## Supported hardware
- `Alienware m15R1`, `Alienware m17R1`

## `alienfan-cli` usage
it's a simple CLI fan control utility for now, providing the same functionality as AWCC (well... a little more).
`m15/17R1` can't control fans directly (well... i'm working about it), so all you can do is set fan boost (More RPM).  
Run `alienfan-cli [lock [boost1 boost2]]`.  
- Without parameters it provide current system temperatures and fans RPM (funny, AWCC only show "percent" - maybe because they don't want to reveal maximal RPMs is about 5000 - that's why this model so hot - cheap slow fans!)
- If lock parameter provided, cli locks/unlocks fan boost (1 - unlock, 0 - lock).
- If both lock and 2 boost provided, cli set fan boost to values provided (well... setting lock to 0 useless in this case).  
Boosts can be from 0 to 100 (in percent), 100% provide about +2000 RPM to current temp-based values.  
Boost1 for CPU fan, Boost2 for GPU one. 

## ToDo:
- [ ] Temperature sensors reading
- [ ] Eliminate "Test mode" requirement
- [ ] Additional hardware support (thanks for ACPI dumps, provided by `alienfx-tools` community!):
  - [ ] `Alienware m15R4`
  - [ ] `Alienware m15R3`
  - [ ] `Alienware 17R3`
  - [ ] `Alienware 13R2`
  - [ ] `Aurora R7, R8` (also FX support for R7 - this model have it into ACPI!)
- [ ] Temp-RPM curves and indirect RPM control
- [ ] CPU power limit control
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


