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

#if (SAMPLE_PROGRAM_NO == 1)
// SAMPLE_PROGRAM_NO  1 : FlashAPI sample
//
// This is a sample program of flash writing.
// Writes to the address set in FLASH_ADDR.
// Erasing is in units of 4096 bytes.

#include "mbed.h"
#include "mbed_drv_cfg.h"

#define FLASH_ADDR  (FLASH_BASE + FLASH_SIZE - 0x100000)

static uint8_t flash_work_buff[256];

static void disp_flash_work_buff(void) {
    for (uint32_t i = 0; i < sizeof(flash_work_buff); i++) {
        if ((i != 0) && ((i & 0x0F) == 0x00)) {
            printf("\r\n");
        }
        printf("%02x ", flash_work_buff[i]);
    }
    printf("\r\n\r\n");
}

int main(void) {
    int ret;
    uint32_t i;
    FlashIAP flash;

    flash.init();

    // read
    memset(flash_work_buff, 0, sizeof(flash_work_buff));
    ret = flash.read(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.read ret=%d\r\n", (int)ret);
    disp_flash_work_buff();

    // erase
    ret = flash.erase(FLASH_ADDR, flash.get_sector_size(FLASH_ADDR));
    printf("flash.erase ret=%d\r\n", (int)ret);

    // read
    memset(flash_work_buff, 0, sizeof(flash_work_buff));
    ret = flash.read(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.read ret=%d\r\n", (int)ret);
    disp_flash_work_buff();

    // write
    for (i = 0; i < sizeof(flash_work_buff); i++) {
        flash_work_buff[i] = i;
    }
    ret = flash.program(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.write ret=%d\r\n", (int)ret);

    // read
    memset(flash_work_buff, 0, sizeof(flash_work_buff));
    ret = flash.read(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.read ret=%d\r\n", (int)ret);
    disp_flash_work_buff();

    while (1);
}

#endif
