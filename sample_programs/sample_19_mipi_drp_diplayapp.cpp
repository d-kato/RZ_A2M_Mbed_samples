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

#if (SAMPLE_PROGRAM_NO == 19)
// SAMPLE_PROGRAM_NO 19 : MIPI, DRP and USBSerial (CDC) sample (use "DisplayApp")
// If you want to know more about SimpleIsp, please refer to: https://github.com/d-kato/RZ_A2M_WebCamera_MIPI
//
// Converts the input image from MIPI camera to YUV image using DRP and outputs to PC display using DisplayApp.
// DisplayApp (Image display on PC display with USB connection)
// https://github.com/d-kato/mbed-gr-libs#displayapp-image-display-on-pc-display-with-usb-connection
// Please refer to the document for details. (RZ_A2M_Mbed\drp-for-mbed\r_drp\doc),
//
// Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_RASPBERRY_PI".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_RASPBERRY_PI"
//        },
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

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
#include "JPEG_Converter.h"
#include "DisplayApp.h"
#include "dcache-control.h"

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640)
#define VIDEO_PIXEL_VW         (480)

#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * 1) + 31u) & ~31u)
#define FRAME_BUFFER_STRIDE_2  (((VIDEO_PIXEL_HW * 2) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

#define DRP_FLG_TILE_ALL       (R_DK2_TILE_0 | R_DK2_TILE_1 | R_DK2_TILE_2 | R_DK2_TILE_3 | R_DK2_TILE_4 | R_DK2_TILE_5)
#define DRP_FLG_CAMER_IN       (0x00000100)

#define WB_DAYLIGHT            (0)
#define WB_CLOUDY              (1)
#define WB_FLUORESCENT         (2)
#define WB_TUNGSTEN            (3)

static DisplayBase Display;
static uint8_t fbuf_bayer[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(128)));
static uint8_t fbuf_yuv[FRAME_BUFFER_STRIDE_2 * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t JpegBuffer[1024 * 64]__attribute((aligned(32)));
static DisplayApp  display_app;

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
    uint32_t tile_no;
    uint32_t set_flgs = 0;

    // Change the operation state of the DRP library notified by the argument to finish
    for (tile_no = 0; tile_no < R_DK2_TILE_NUM; tile_no++) {
        if (drp_lib_id[tile_no] == id) {
            set_flgs |= (1 << tile_no);
        }
    }
    drpTask.flags_set(set_flgs);
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

static void button_fall(void) {
    if (isp_wb_mode_req < WB_TUNGSTEN) {
        isp_wb_mode_req++;
    } else {
        isp_wb_mode_req = WB_DAYLIGHT;
    }
}

static void drp_task(void) {
    JPEG_Converter  Jcu;
    JPEG_Converter::bitmap_buff_info_t bitmap_buff_info;
    JPEG_Converter::encode_options_t   encode_options;
    size_t encode_size;

    EasyAttach_Init(Display);
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

    InterruptIn button(USER_BUTTON0);
    button.fall(&button_fall);

    bitmap_buff_info.width              = VIDEO_PIXEL_HW;
    bitmap_buff_info.height             = VIDEO_PIXEL_VW;
    bitmap_buff_info.format             = JPEG_Converter::WR_RD_YCbCr422;
    bitmap_buff_info.buffer_address     = (void *)fbuf_yuv;
    encode_options.encode_buff_size     = sizeof(JpegBuffer);
    encode_options.input_swapsetting    = JPEG_Converter::WR_RD_WRSWA_32_16BIT;

    while (true) {
        ThisThread::flags_wait_all(DRP_FLG_CAMER_IN);
        if (isp_wb_mode_req != isp_wb_mode) {
            isp_wb_mode = isp_wb_mode_req;
            switch (isp_wb_mode) {
                case WB_DAYLIGHT:
                    param_isp.gain_r = 0x1266;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x1359;
                    break;
                case WB_CLOUDY:
                    param_isp.gain_r = 0x1400;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x11CA;
                    break;
                case WB_FLUORESCENT:
                    param_isp.gain_r = 0x1000;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x163D;
                    break;
                case WB_TUNGSTEN:
                    param_isp.gain_r = 0x0E66;
                    param_isp.gain_g = 0x0CB0;
                    param_isp.gain_b = 0x1876;
                    break;
            }
        }
        R_DK2_Start(drp_lib_id[0], (void *)&param_isp, sizeof(r_drp_simple_isp_t));

        ThisThread::flags_wait_all(DRP_FLG_TILE_ALL);
        dcache_invalid(JpegBuffer, sizeof(JpegBuffer));
        if (Jcu.encode(&bitmap_buff_info, JpegBuffer, &encode_size, &encode_options) == JPEG_Converter::JPEG_CONV_OK) {
            display_app.SendJpeg(JpegBuffer, (int)encode_size);
        }
    }
}

int main(void) {
    // Start DRP task
    drpTask.start(callback(drp_task));

    wait(osWaitForever);
}

#endif
