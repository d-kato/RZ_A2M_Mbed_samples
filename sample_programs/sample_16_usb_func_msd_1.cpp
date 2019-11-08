/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
#include "sample_select.h"

#if (SAMPLE_PROGRAM_NO == 16)
// SAMPLE_PROGRAM_NO 16 : USBMSD and FlashAPI sample
//
// USBMSD and FlashAPI sample.
// It is a sample program that can read and write serial flash from the PC via Filesystem.
// By connecting the board and the PC with a USB cable, you can refer inside the device as a mass storage device.
// (The format is required when connecting for the first time.)
// Part of the flash address is used as storage.
// The first 1MB is used as the program area, and the rest is used as the storage.
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBMSD.h"
#include "FlashIAPBlockDevice.h"
#include "mbed_drv_cfg.h"

int main() {
    FlashIAPBlockDevice flashiap_bd(FLASH_BASE + 0x100000, FLASH_SIZE - 0x100000);
    USBMSD usb(&flashiap_bd);

    while (true) {
        usb.process();
    }
}

#endif
