# x5-initializer --- Initializer for Snapdragon X5 LTE modems to use them in Linux.

Set of scripts and programs for initialize Snapdragon X5 LTE modems.

Content:
- udev.d - UDEV scripts for configure modem to provide QMI interface
- device-download-mode-changer - program for switch modem to firmware download mode
- copy_firmware.sh - Script to flash new firmware to modem


x5_initializer is an initializer for Qualcomm X5 LTE modems. These devices appear in an unconfigured
state when power is applied and require firmware to be loaded and configuration to be activasted
before they can be used as modems.

Building:
```shell
pushd device_download_mode_changer
cmake && make
popd
```

Now you need the modem firmware. This can be obtained from a Windows install - alternatively it may
be possible to download from your vendor's site and extracted with wine. On my install, these files
could be found in a `C:\Program Files (x86)\HP lt4120 Snapdragon X5 LTE\Image` directory.

Please don't ask me for firmware. It's copyright Qualcomm and I can't redistribute it.

Author:

This code was written by Aliaksandr Barouski <alex.borovsky@gmail.com> and is released under the
terms of version 3 of the GNU General Public License. It is based on code of libqmi-glib. The code
was written by examining firmware upload tool's logs in Windows and dumping USB traffic - the
Qualcomm drivers or firmware have not been reverse engineered or disassembled in any way.
