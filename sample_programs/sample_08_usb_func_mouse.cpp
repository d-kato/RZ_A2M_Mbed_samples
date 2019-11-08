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

#if (SAMPLE_PROGRAM_NO == 8)
// SAMPLE_PROGRAM_NO  8 : USBMouse sample
//
// The USBMouse interface is used to emulate a mouse over the USB port. 
// You can choose relative or absolute co-ordinates, and send clicks, button state and scroll wheel movements.
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBMouse.h"

int main() {
    USBMouse mouse;
    int16_t x = 0;
    int16_t y = 0;
    int32_t radius = 10;
    int32_t angle = 0;

    while (1) {
        x = cos((double)angle*3.14/180.0)*radius;
        y = sin((double)angle*3.14/180.0)*radius;

        mouse.move(x, y);
        angle += 3;
        wait(0.001);
    }
}

#endif
