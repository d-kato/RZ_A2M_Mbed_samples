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

#if (SAMPLE_PROGRAM_NO == 15)
// SAMPLE_PROGRAM_NO 15 : CEU, LCD and PWM sample
//
// Display camera image on LCD
// Pressing SW3 changes the LCD backlight. (PWM)
//
// Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_MT9V111", "CAMERA_OV7725", "CAMERA_OV5642" or "CAMERA_WIRELESS_CAMERA".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_XXXXXXXX"
//        },
//
// Please set the value of "lcd-type" in "mbed_app.json" to "GR_PEACH_xxx", "GR_LYCHEE_xxx" or "RZ_A2M_EVB_RSK_TFT".
//
//        "lcd-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "RZ_A2M_EVB_RSK_TFT"
//        },

#if defined(TARGET_SEMB1402) || defined(TARGET_RZ_A2M_SBEV)
#error "LCD is not supported."
#endif

#include "mbed.h"
#include "EasyAttach_CameraAndLCD.h"

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640)
#define VIDEO_PIXEL_VW         (480)

#define DATA_SIZE_PER_PIC      (2u)
#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

DisplayBase Display;

static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));

static void Start_Video_Camera(void) {
    // Video capture setting (progressive form fixed)
    Display.Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)user_frame_buffer0,
        FRAME_BUFFER_STRIDE,
        DisplayBase::VIDEO_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        VIDEO_PIXEL_VW,
        VIDEO_PIXEL_HW
    );
    EasyAttach_CameraStart(Display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;

#if (LCD_PIXEL_HEIGHT > VIDEO_PIXEL_VW)
    rect.vs = (LCD_PIXEL_HEIGHT - VIDEO_PIXEL_VW) / 2;  // centering
#else
    rect.vs = 0;
#endif
    rect.vw = VIDEO_PIXEL_VW;
#if (LCD_PIXEL_WIDTH > VIDEO_PIXEL_HW)
    rect.hs = (LCD_PIXEL_WIDTH - VIDEO_PIXEL_HW) / 2;   // centering
#else
    rect.hs = 0;
#endif
    rect.hw = VIDEO_PIXEL_HW;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)user_frame_buffer0,
        FRAME_BUFFER_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

    ThisThread::sleep_for(50);
    EasyAttach_LcdBacklight(true);
}

static void button_fall(void) {
    static uint32_t backlight_val = 10;

    if (backlight_val > 0) {
        backlight_val--;
    } else {
        backlight_val = 10;
    }
    EasyAttach_LcdBacklight((float)(0.1f * backlight_val));
}

int main(void) {
    EasyAttach_Init(Display);
    Start_LCD_Display();
    Start_Video_Camera();

    InterruptIn button(USER_BUTTON0);
    button.fall(&button_fall);

    wait(osWaitForever);
}

#endif
