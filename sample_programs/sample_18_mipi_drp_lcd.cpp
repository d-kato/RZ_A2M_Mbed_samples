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

#if (SAMPLE_PROGRAM_NO == 18)
// SAMPLE_PROGRAM_NO 18 : MIPI, DRP and LCD sample
// If you want to know more about SimpleIsp, please refer to: https://github.com/d-kato/RZ_A2M_WebCamera_MIPI
//
// Converts the input image from MIPI camera to YUV image using DRP and outputs to display.
// Please refer to the document for details. (RZ_A2M_Mbed\drp-for-mbed\r_drp\doc),
//
// Please set the value of "camera-type" in "mbed_app.json" to null or "CAMERA_RASPBERRY_PI".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": null
//        },
//
//        or
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_RASPBERRY_PI"
//        },
//
// Requirements
//   RZ/A2M Evaluation Board Kit : Display Output Board
//   SBEV-RZ/A2M                 : LVDS To HDMI Board
//   SEMB1402                    : LVDS To HDMI Board

#if !defined(TARGET_RZ_A2XX)
#error "DRP and MIPI are not supported."
#endif
#if MBED_CONF_APP_CAMERA_TYPE != CAMERA_RASPBERRY_PI
#error Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_RASPBERRY_PI" and build.
#endif

#include "mbed.h"
#include "EasyAttach_CameraAndLCD.h"
#include "AsciiFont.h"
#include "r_dk2_if.h"
#include "r_drp_simple_isp.h"

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         LCD_PIXEL_WIDTH
#define VIDEO_PIXEL_VW         LCD_PIXEL_HEIGHT

#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * 1) + 31u) & ~31u)
#define FRAME_BUFFER_STRIDE_2  (((VIDEO_PIXEL_HW * 2) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

#define OVL_WIDTH              (AsciiFont::CHAR_PIX_WIDTH * 2 * 11)
#define OVL_STRIDE             (((OVL_WIDTH * 1) + 31u) & ~31u)
#define OVL_HEIGHT             (AsciiFont::CHAR_PIX_HEIGHT * 2)

#define DRP_FLG_CAMER_IN       (0x00000100)

#define WB_DAYLIGHT            (0)
#define WB_CLOUDY              (1)
#define WB_FLUORESCENT         (2)
#define WB_TUNGSTEN            (3)

static DisplayBase Display;
static uint8_t fbuf_bayer[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(128)));
static uint8_t fbuf_yuv[FRAME_BUFFER_STRIDE_2 * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t fbuf_overlay[OVL_STRIDE * OVL_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));

static r_drp_simple_isp_t param_isp __attribute((section("NC_BSS")));
static uint8_t drp_lib_id[R_DK2_TILE_NUM] = {0};
static Thread drpTask(osPriorityHigh);
static uint32_t isp_wb_mode_req;
static uint32_t isp_wb_mode;

static const uint32_t clut_data_resut[] = {0x00000000, 0xff00ff00};  // ARGB8888

static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type) {
    drpTask.flags_set(DRP_FLG_CAMER_IN);
}

static void cb_drp_finish(uint8_t id) {
    // do nothing
}

static void Start_Video_Camera(void) {
    // Video capture setting (progressive form fixed)
    Display.Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)fbuf_bayer,
        FRAME_BUFFER_STRIDE,
        DisplayBase::VIDEO_FORMAT_RAW8,
        DisplayBase::WR_RD_WRSWA_NON,
        VIDEO_PIXEL_VW,
        VIDEO_PIXEL_HW
    );
    EasyAttach_CameraStart(Display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;
    DisplayBase::clut_t clut_param;

    rect.vs = 0;
    rect.vw = VIDEO_PIXEL_VW;
    rect.hs = 0;
    rect.hw = VIDEO_PIXEL_HW;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)fbuf_yuv,
        FRAME_BUFFER_STRIDE_2,
        DisplayBase::GRAPHICS_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16_8BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

    memset(fbuf_overlay, 0, sizeof(fbuf_overlay));
    clut_param.color_num = sizeof(clut_data_resut) / sizeof(uint32_t);
    clut_param.clut = clut_data_resut;

    rect.vs = 5;
    rect.vw = OVL_HEIGHT;
    rect.hs = 5;
    rect.hw = OVL_WIDTH;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_2,
        (void *)fbuf_overlay,
        OVL_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_CLUT8,
        DisplayBase::WR_RD_WRSWA_32_16_8BIT,
        &rect,
        &clut_param
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_2);

    ThisThread::sleep_for(50);
    EasyAttach_LcdBacklight(true);
}

static void button_fall(void) {
    if (isp_wb_mode_req < WB_TUNGSTEN) {
        isp_wb_mode_req++;
    } else {
        isp_wb_mode_req = WB_DAYLIGHT;
    }
}

static void drp_task(void) {
    AsciiFont ascii_font(fbuf_overlay, OVL_WIDTH, OVL_HEIGHT, OVL_STRIDE, 1);
    char str[64];

    EasyAttach_Init(Display);
    Start_LCD_Display();
    // Interrupt callback function setting (Field end signal for recording function in scaler 0)
    Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);
    Start_Video_Camera();

    R_DK2_Initialize();

    /* Load DRP Library                 */
    /*        +-----------------------+ */
    /* tile 0 |                       | */
    /*        +                       + */
    /* tile 1 |                       | */
    /*        +                       + */
    /* tile 2 |                       | */
    /*        + SimpleIsp bayer2yuv_6 + */
    /* tile 3 |                       | */
    /*        +                       + */
    /* tile 4 |                       | */
    /*        +                       + */
    /* tile 5 |                       | */
    /*        +-----------------------+ */
    R_DK2_Load(g_drp_lib_simple_isp_bayer2yuv_6,
               R_DK2_TILE_0,
               R_DK2_TILE_PATTERN_6, NULL, &cb_drp_finish, drp_lib_id);
    R_DK2_Activate(0, 0);

    memset(&param_isp, 0, sizeof(param_isp));
    param_isp.src    = (uint32_t)fbuf_bayer;
    param_isp.dst    = (uint32_t)fbuf_yuv;
    param_isp.width  = VIDEO_PIXEL_HW;
    param_isp.height = VIDEO_PIXEL_VW;
    param_isp.gain_r = 0x1266;
    param_isp.gain_g = 0x0CB0;
    param_isp.gain_b = 0x1359;

    isp_wb_mode_req = WB_DAYLIGHT;
    isp_wb_mode = WB_DAYLIGHT;
    sprintf(str, "Daylight");
    ascii_font.DrawStr(str, 0, 0 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 0, 1, 2);

    InterruptIn button(USER_BUTTON0);
    button.fall(&button_fall);

    while (true) {
        ThisThread::flags_wait_all(DRP_FLG_CAMER_IN);
        if (isp_wb_mode_req != isp_wb_mode) {
            isp_wb_mode = isp_wb_mode_req;
            switch (isp_wb_mode) {
                case WB_DAYLIGHT:
                    param_isp.gain_r = 0x1266;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x1359;
                    sprintf(str, "Daylight");
                    break;
                case WB_CLOUDY:
                    param_isp.gain_r = 0x1400;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x11CA;
                    sprintf(str, "Cloudy");
                    break;
                case WB_FLUORESCENT:
                    param_isp.gain_r = 0x1000;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x163D;
                    sprintf(str, "Fluorescent");
                    break;
                case WB_TUNGSTEN:
                    param_isp.gain_r = 0x0E66;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x1876;
                    sprintf(str, "Tungsten");
                    break;
            }
            memset(fbuf_overlay, 0, sizeof(fbuf_overlay));
            ascii_font.DrawStr(str, 0, 0 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 0, 1, 2);
        }
        R_DK2_Start(drp_lib_id[0], (void *)&param_isp, sizeof(r_drp_simple_isp_t));
    }
}

int main(void) {
    // Start DRP task
    drpTask.start(callback(drp_task));

    wait(osWaitForever);
}

#endif
