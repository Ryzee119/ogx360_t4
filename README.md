## ogx360_t4
![Build](https://github.com/Ryzee119/ogx360_t4/workflows/Build/badge.svg)

A port of https://github.com/Ryzee119/ogx360.git, a project to use modern USB game controllers on the original Xbox console to the Teensy4.1.  

This has a new USB Host Stack, USB Device Stack and significantly more processing power which opens up alot more possibilities.

Finally, hardly any soldering is required.

## Currently can emulate
- Duke Standard Controller
- Official Xbox Memory Unit (XMU)
- Steel Battalion Controller
- DVD Movie Playback IR Dongle

## Needed Parts
| Qty | Part Description | Link |
|--|--|--|
| 1 | Teensy 4.1 | https://www.pjrc.com/store/teensy41.html |
| 1 | USB Host Cable | https://www.pjrc.com/store/cable_usb_host_t36.html |
| 1 | 0.1" Pin Header | https://www.pjrc.com/store/header_24x1.html |
| 1 | SD Card | For XMU emulation. Clean format FAT32. I had to use this https://www.sdcard.org/downloads/formatter/ on my old SD cards |
| 1 | Xbox to MicroUSB | [ChimericSystems](https://chimericsystems.com/products/console-usb-adapter) or [Alibaba (Large MOQ!)](https://www.alibaba.com/product-detail/for-XBOX-MicroUSB-Cable-for-Xbox_62222784495.html) or DIY |

## Duke Controller Emulation: Supported Controllers
- Bluetooth 8bitdo/compatible controllers via the [8BitDo Wireless USB Adapter](https://www.8bitdo.com/wireless-usb-adapter/) or [Rev 2](https://www.8bitdo.com/wireless-usb-adapter-2/).
- Wired 8bitdo controllers when they are started in X-input mode.
- Xbox S/X Wired
- Xbox One Wired (Genuine / PDP)
- Xbox 360 Wired
- Xbox 360 Wireless (Via PC USB Receiver)
- PS4 Wired

## Xbox Memory Unit Emulation (XMU): Supported Memory Interfaces
- 256kB RAM disk for testing only.
- SD Card installed into the Teensy4.1 SD Card slot.

## Steel Battalion Controller Emulation: Supported Interfaces
- Keyboard and Mouse See [this file](/src/steelbattalion.cpp) for mapping. Please improve!

## DVD Playback IR Dongle
- To play DVDs on a Xbox console, place a file called `dvd_rom.bin` at the root of a FAT32 formatted SD card then insert it into the Teensy before power up. `dvd_rom.bin` [must be dumped](https://github.com/Ryzee119/Dongle_Dumper) from a genuine IR dongle.
- Input is not yet fully implemented. Currently only Duke and Xbox360 D-Pad is mapped to the IR remote dpad for testing. This is very simple to fix though if needed.

## Compile
### CLI (Requires python and python-pip)
Configure platformio.ini to enable XMU support or Steel Battation etc.
```
git clone https://github.com/Ryzee119/ogx360_t4.git --recursive
python -m pip install --upgrade pip
pip install platformio
cd ogx360_t4
# Build standard Duke interface
platformio run -e DUKE
```
### Visual Studio Code
* Download and install [Visual Studio Code](https://code.visualstudio.com/).
* Install the [PlatformIO IDE](https://platformio.org/platformio-ide) plugin.
* Clone this repo recursively `git clone https://github.com/Ryzee119/ogx360_t4.git --recursive`
* In Visual Studio Code `File > Open Folder... > ogx360_t4`
* Hit build on the Platform IO toolbar (`✓`).

## Program
### Teensy (using Teensy Loader)
* Connect the Teensy to your PC using a MicroUSB cable.
* Run the [Teensy Loader Application](https://www.pjrc.com/teensy/loader.html).

### Teensy (using Visual Studio Code)
* Setup Visual Studio Code as per the Compile instructions.
* Hit the program button on the Platform IO toolbar (`→`).

<img src="./images/ogx360_t4.jpg" alt="usagephoto" width="80%"/>  
