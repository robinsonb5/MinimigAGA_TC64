# ARCHIVED

This repo is now archived.
Up-to-date development on this core will happen at
https://github.com/minimig-dev/MinimigAGA-MiST-TC64

# Minimig AGA
For Turbo Chameleon TC64, MiST and other platforms.
(This core should be easily portable to any FPGA board with VGA out, PS/2 in, SD-card, about 25,000 logic elements, and a 16-bit wide SDRAM supporting 13x9 layout for 32 megabytes of RAM.)

### Foreword

[minimig](http://en.wikipedia.org/wiki/Minimig) (short for Mini Amiga) is an open source re-implementation of an Amiga using a field-programmable gate array (FPGA). Original minimig author is Dennis van Weeren.

[Amiga](http://en.wikipedia.org/wiki/Amiga_500) was an amazing personal computer, announced around 1984, which - at the time - far surpassed any other personal computer on the market, with advanced graphic & sound capabilities, not to mention its great OS with preemptive multitasking capabilities.

This minimig variant has been upgraded with [AGA chipset](http://en.wikipedia.org/wiki/Amiga_Advanced_Graphics_Architecture) capabilites, which allows it to emulate the latest Amiga models ([Amiga 1200](http://en.wikipedia.org/wiki/Amiga_1200), [Amiga 4000](http://en.wikipedia.org/wiki/Amiga_4000) and (partially) [Amiga CD32](http://en.wikipedia.org/wiki/Amiga_CD32)). Ofcourse it also supports previous OCS/ECS Amigas like [Amiga 500](http://en.wikipedia.org/wiki/Amiga_500), [Amiga 600](http://en.wikipedia.org/wiki/Amiga_600) etc.


## Core features supported

* chipset variants : OCS, ECS, AGA
* chipRAM : 0.5MB - 2.0MB
* slowRAM : 0.0MB - 1.5MB
* fastRAM : 0.0MB - 24MB
* CPU core : 68000, 68010, 68020
* kickstart : 1.2 - 3.1 (256kB, 512kB & 1MB kickstart ROMs currently supported)
* HRTmon with custom registers mirror
* floppy disks : 1-4 floppies (supports ADF floppy image format), with normal & turbo speeds
* hard disks : 1-2 hard disk images (supports whole disk images, partition images, using whole SD card and using SD card partition)
* video standard : PAL / NTSC
* supports normal & scandoubled video output (15kHz / 30kHz) - can be used with a monitor or a TV with a SCART cable
* peripherals : real Amiga / C64 joysticks connected to C64 joystick ports, CDTV infra-red controllers, PS/2 keyboards,
PS/2 mice, Turbo Chameleon Docking Station for extra joysticks or real Amiga mouse, and MIDI in / out
* supports basic retargetable graphics with a P96 driver
* has an implementation of the Akiko chunky to planar converter
* has an extra audio channel which can be used from the Amiga to play CD-quality WAV files, or used on some platforms to emulate floppy drive sounds.


## Usage

### Hardware
To use this minimig core, you will at the minimum need an SD/SDHC card, formatted with the FAT32 filesystem, a PS/2 keyboard and a compatible monitor / TV. Joysticks & mouse can be emulated on the keyboard. You will probably want to attach a set of speakers of headphones, a real Amiga or USB mouse and a real Amiga joystick.

### Software

To use the core, you will also need a Kickstart ROM image file, which you can obtain by copying Kickstart ROM IC from your actual Amiga, or by buying an [Amiga Forever](http://www.amigaforever.com/) software pack. The Kickstart image should be placed on the root of the SD card with the name KICK.ROM. Minimig also supports the [AROS](http://aros.sourceforge.net/) kickstart ROM replacement.

The minimig can read any ADF floppy images you place on the SD card. I recommend at least Workbench 1.3 or 3.1 (AmigaOS), some of the Amigas great games (I recommend Ruff'n'Tumble) or some of the amazing demos from the vast Amiga demoscene (like State of the Art from Spaceballs).

The minimig can also use HDF harddisk images, which can be created with [WinUAE](http://www.winuae.net/).

### Recommended minimig config

* for ECS games / demos : CPU = 68000, Turbo=NONE, Chipset=ECS, chipRAM=0.5MB, slowRAM=0.5MB, Kickstart 1.3
* for AGA games / demos : CPU = 68020, Turbo=NONE, Chipset=AGA, chipRAM=2MB, slowRAM=0MB, fastRAM=24MB, Kickstart 3.1
For Workbench usage, you can try turning TURBO=BOTH for a little speed increase.

### Controlling minimig

Keyboard special keys:

* F12         - OSD menu
* F11         - start monitor (HRTmon) if HRTmon is enabled in OSD menu (otherwise F11 is the Amiga HELP key)
* ScrollLock  - toggle keyoard only / mouse / joystick 1 / joystick 2 emulation on the keyboard (direction keys + LCTRL)


## Links & more info

Rok Krajnc's page [somuch.guru](http://somuch.guru/).

Further info about minimig can be found on the [Minimig Discussion Forum](http://www.minimig.net/).

The Turbo Chameleon 64 - [Individual Computers]http://wiki.icomp.de/wiki/Chameleon

MiST board support & other cores on the [MiST Project Page](https://github.com/mist-devel/mist-board/wiki)


## Credits
This project contains code written by:
* Jakub Bednarski
* Sascha Boing
* Tobias Gubener
* Till Harbaum
* Rok Krajnc
* Alastair M. Robinson
* Gyorgy Szombathelyi
* Dennis van Weeren

All code is copyright © 2005 - 2020 and the property of its respective authors.


## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


## Building minimig-mist from sources

* checkout the source 
* download / install [Altera Quartus II](https://dl.altera.com/?edition=web) (latest supported version for Cyclone III FPGA device used on the MiST board is 13.1; I'm still using version 10.1SP1)
* build the core using Quartus GUI (project file in fpga/mist)
* place the minimig-mist.rbf files on the root of your SD card (optionally, rename minimig-mist.rbf to core.rbf to make it the default core)
* don't forget to place kickstart ROM of your choosing on the root of the SD card (these are still copyrighted, so either copy the ROM from your real Amiga, or buy AmigaForever)
* place some ADF (floppy disk images) of your favourite games / demos / programs on your SD card
* optionally place minimig.bal, minimig.art & minimig.cop files on the root of your SD card for a nice bootup animation
* enjoy minimig! :)


## Sources

This sourcecode is based on Rok's previous project ([minimig-de1](https://github.com/rkrajnc/minimig-de1)), and it continues from there. It was split into a new project to allow changes that would never fit in the FPGA on the DE1 board.

Original minimig sources from Dennis van Weeren with updates by Jakub Bednarski are published on [Google Code](http://code.google.com/p/minimig/).

Some minimig updates are published on the [Minimig Discussion Forum](http://www.minimig.net/), done by Sascha Boing.

ARM firmware updates and minimig-tc64 port changes by Christian Vogelsang ([minimig_tc64](https://github.com/cnvogelg/minimig_tc64)) and A.M. Robinson ([minimig_tc64](https://github.com/robinsonb5/minimig_tc64)).

MiST board & firmware by Till Harbaum ([MiST](https://code.google.com/p/mist-board/)).

TG68K.C core by Tobias Gubener.

