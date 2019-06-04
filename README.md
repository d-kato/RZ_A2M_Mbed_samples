# RZ_A2M_Mbed_samples
This is a collection of sample programs that work on RZ/A2M boards.  
You can try Mbed OS for RZ/A2M with the following board.
- [RZ/A2M Evaluation Board Kit](https://www.renesas.com/jp/en/products/software-tools/boards-and-kits/eval-kits/rz-a2m-evaluation-board-kit.html)  
- [SBEV-RZ/A2M](http://www.shimafuji.co.jp/products/1486)  
- [SEMB1402](http://www.shimafuji.co.jp/products/1505)  


## Overview
You can try each sample program by changing the following macro. (main.cpp)  
```cpp
#define SAMPLE_PROGRAM_NO  0
```

| No.| Description                                                 | A | B | C | D | E |
|:---|:------------------------------------------------------------|:--|:--|:--|:--|:--|
|  0 | DigitalOut, InterruptIn, RTC, Timer and AnalogI             | x | x | x | x | x |
|  1 | FlashAPI sample                                             | x | x | x | x | x |
|  2 | SSIF loop back sample                                       | x | x | x |   |   |
|  3 | SPDIF loop back sample                                      |   |   | x |   |   |
|  4 | SSIF wav playback sample (use USB memory or SD card)        | x | x | x |   |   |
|  5 | SPDIF wav playback sample (use USB memory or SD card)       |   |   | x |   |   |
|  6 | LCD, Touch panel and JCU sample (use USB memory or SD card) | x | x | x |   |   |
|  7 | USBSerial (CDC) sample                                      | x | x | x | x | x |
|  8 | USBMouse sample                                             | x | x | x | x | x |
|  9 | USBKeyboard sample                                          | x | x | x | x | x |
| 10 | USBMIDI sample                                              | x | x | x | x | x |
| 11 | USBAudio sample                                             | x | x | x | x | x |
| 12 | USBAudio and SSIF sample                                    | x | x | x |   |   |
| 13 | Ether HTTP sample                                           | x |   | x | x |   |
| 14 | Ether HTTPS sample                                          | x |   | x | x |   |
| 15 | CEU, LCD and PWM sample                                     |   |   | x |   |   |
| 16 | USBMSD and FlashAPI sample                                  | x | x | x | x | x |
| 17 | USBMSD and FlashAPI sample advanced version                 | x | x | x | x | x |
| 18 | MIPI, DRP and LCD sample                                    |   |   | x | x | x |
| 19 | MIPI, DRP and USBSerial (CDC) sample (use "DisplayApp")     |   |   | x | x | x |
| 20 | DRP Dynamic Loading Sample                                  |   |   | x | x | x |

A : [GR-PEACH](https://os.mbed.com/platforms/Renesas-GR-PEACH/) (RZ/A1H)  
B : [GR-LYCHEE](https://os.mbed.com/platforms/Renesas-GR-LYCHEE/) (RZ/A1LU)  
C : [RZ/A2M Evaluation Board Kit](https://www.renesas.com/jp/en/products/software-tools/boards-and-kits/eval-kits/rz-a2m-evaluation-board-kit.html) (RZ/A2M)  
D : [SBEV-RZ/A2M](http://www.shimafuji.co.jp/products/1486) (RZ/A2M)  
E : [SEMB1402](http://www.shimafuji.co.jp/products/1505) (RZ/A2M)  

## Board setting
- RZ/A2M Evaluation Board Kit  
Please set the dip switch SW6 of the SUB board as follows.
```
SW6-1  OFF  
SW6-2  OFF  
SW6-3  OFF  
SW6-4  OFF  
SW6-5  ON  
SW6-6  ON  
SW6-7  ON  
SW6-8  OFF  
SW6-9  OFF  
SW6-10 OFF  
```

## About custom boot loaders
This sample uses a custom boot loader, and you can drag & drop the "xxxx_application.bin" file to write the program.  

1. Hold down ``SW3`` and press the reset button. (Or turn on the power.)  
2. Connect the USB cable to the PC, you can find the ``MBED`` directory.  
3. Drag & drop ``xxxx_application.bin`` to the ``MBED`` directory.  
4. When writing is completed, press the reset button.  

**Attention!**  
For the first time only, you need to write a custom bootloader as following.  
[How to write a custom boot loader](https://github.com/d-kato/bootloader_d_n_d)  


## Development environment
You can use ``Mbed CLI (CUI)`` or ``Mbed Studio (GUI)``. Choose your preferred development environment.  

### When using Mbed CLI (CUI)
You can use ``GCC``, ``Arm Compiler 5``, ``Arm Compiler 6`` and ``IAR``. A license is required to use a compiler other than the ``GCC`` compiler.  
Information of Mbed CLI that includes install&quick start guide is as the following.  
[Installation](https://github.com/ARMmbed/mbed-cli/blob/1.8.3/README.md#installation)  

How to import and build this sample  
```
$ cd <projects directory>
$ mbed import https://github.com/d-kato/RZ_A2M_Mbed_samples
$ cd RZ_A2M_Mbed_samples
$ mbed compile -m <TARGET> -t GCC_ARM --profile debug
```

Set the following to ``<TARGET>``.  
- RZ/A2M Evaluation Board Kit : ``RZ_A2M_EVB``  
- SBEV-RZ/A2M : ``RZ_A2M_SBEV``  
- SEMB1402 : ``SEMB1402``  

See [About custom boot loaders](#about-custom-boot-loaders) for program writing.  
See [How to debug using e2studio](#how-to-debug-using-e2studio) for debugging.  


### When using Mbed Studio (GUI)
You can use ``Arm Compiler 6`` included with Mbed Studio for free.  
Information of Mbed Studio that includes install&quick start guide is as the following.  
[Installation](https://os.mbed.com/studio/)  

How to import and build this sample  
![](docs/img/how_to_use_mbed_stusio_1.png)  
![](docs/img/how_to_use_mbed_stusio_2.png)  
![](docs/img/how_to_use_mbed_stusio_3.png)  
![](docs/img/how_to_use_mbed_stusio_4.png)  


**Attention!**  
You can not debug using Mbed Studio. Use only for build purposes. Debug the elf file created by Mbed Studio using e2studio.  

See [About custom boot loaders](#about-custom-boot-loaders) for program writing.  
See [How to debug using e2studio](#how-to-debug-using-e2studio) for debugging.  


## Terminal setting
If you want to confirm the serial communication the terminal soft on your PC, please specify the below values.  
You can change the baud rate by ``platform.stio-baud-rate`` of ``mbed_app.json``.  

|             |         |
|:------------|:--------|
| Baud rate   | 115,200 |
| Data        | 8bit    |
| Parity      | none    |
| Stop        | 1bit    |
| Flow control| none    |

**Attention!**  
``SBEV-RZ/A2M`` and ``SEMB1402`` use the RZ/A2M's USB as the terminal. To use USB for other purposes, delete ``OVERRIDE_CONSOLE_USBSERIAL`` macro in ``mbed_app.json`` file.


## How to debug using e2studio
Download [e2studio 7.4.0 or lator](https://www.renesas.com/eu/en/products/software-tools/tools/ide/e2studio.html), and install. (Debugger : J-Link Base)  
Connect the J-Link to your board.  
![](docs/img/how_to_debug_using_e2studio_1.png)  
![](docs/img/how_to_debug_using_e2studio_2.png)  
![](docs/img/how_to_debug_using_e2studio_3.png)  
![](docs/img/how_to_debug_using_e2studio_4.png)  
![](docs/img/how_to_debug_using_e2studio_5.png)  
![](docs/img/how_to_debug_using_e2studio_6.png)  
![](docs/img/how_to_debug_using_e2studio_7.png)  
![](docs/img/how_to_debug_using_e2studio_8.png)  
![](docs/img/how_to_debug_using_e2studio_9.png)  
![](docs/img/how_to_debug_using_e2studio_10.png)  
