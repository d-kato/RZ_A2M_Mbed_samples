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

#if (SAMPLE_PROGRAM_NO == 20)
// SAMPLE_PROGRAM_NO 20 : DRP Dynamic Loading Sample
//
// Detects the edges of the input image from MIPI camera by Canny method using DRP and outputs to display.
// Please refer to the document for details. (RZ_A2M_Mbed\drp-for-mbed\r_drp\doc)
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
#include "dcache-control.h"
#include "AsciiFont.h"
#include "r_dk2_if.h"
#include "r_drp_bayer2grayscale.h"
#include "r_drp_median_blur.h"
#include "r_drp_canny_calculate.h"
#include "r_drp_canny_hysterisis.h"

#define RAM_TABLE_DYNAMIC_LOADING   1

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640)
#define VIDEO_PIXEL_VW         (480)

#define DATA_SIZE_PER_PIC      (1u)
#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

#define DRP_FLG_TILE_ALL       (R_DK2_TILE_0 | R_DK2_TILE_1 | R_DK2_TILE_2 | R_DK2_TILE_3 | R_DK2_TILE_4 | R_DK2_TILE_5)
#define DRP_FLG_CAMER_IN       (0x00000100)

static DisplayBase Display;
static uint8_t fbuf_bayer[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(128)));
static uint8_t fbuf_work0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t fbuf_work1[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t fbuf_clat8[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t fbuf_overlay[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
static uint8_t drp_work_buf[((FRAME_BUFFER_STRIDE * ((FRAME_BUFFER_HEIGHT / 3) + 2)) * 2) * 3]__attribute((section("NC_BSS")));
static uint8_t nc_memory[512] __attribute((section("NC_BSS")));
static uint8_t drp_lib_id[R_DK2_TILE_NUM] = {0};
static Thread drpTask(osPriorityHigh);

#if RAM_TABLE_DYNAMIC_LOADING
static uint8_t ram_drp_lib_bayer2grayscale[sizeof(g_drp_lib_bayer2grayscale)]__attribute((aligned(32)));
static uint8_t ram_drp_lib_median_blur[sizeof(g_drp_lib_median_blur)]__attribute((aligned(32)));
static uint8_t ram_drp_lib_canny_calculate[sizeof(g_drp_lib_canny_calculate)]__attribute((aligned(32)));
static uint8_t ram_drp_lib_canny_hysterisis[sizeof(g_drp_lib_canny_hysterisis)]__attribute((aligned(32)));
#endif

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

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;
    DisplayBase::clut_t clut_param;

    rect.vs = 0;
    rect.vw = VIDEO_PIXEL_VW;
    rect.hs = 0;
    rect.hw = VIDEO_PIXEL_HW;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)fbuf_clat8,
        FRAME_BUFFER_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_CLUT8,
        DisplayBase::WR_RD_WRSWA_32_16_8BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

    memset(fbuf_overlay, 0, sizeof(fbuf_overlay));
    clut_param.color_num = sizeof(clut_data_resut) / sizeof(uint32_t);
    clut_param.clut = clut_data_resut;

    rect.vs = 0;
    rect.vw = VIDEO_PIXEL_VW;
    rect.hs = 0;
    rect.hw = VIDEO_PIXEL_HW;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_2,
        (void *)fbuf_overlay,
        FRAME_BUFFER_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_CLUT8,
        DisplayBase::WR_RD_WRSWA_32_16_8BIT,
        &rect,
        &clut_param
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_2);

    ThisThread::sleep_for(50);
    EasyAttach_LcdBacklight(true);
}

static void drp_task(void) {
    uint32_t idx;
    uint32_t drp_time[8];
    char str[64];
    Timer t;
    AsciiFont ascii_font(fbuf_overlay, VIDEO_PIXEL_HW, VIDEO_PIXEL_VW, FRAME_BUFFER_STRIDE, DATA_SIZE_PER_PIC);

    EasyAttach_Init(Display);
    Start_LCD_Display();
    // Interrupt callback function setting (Field end signal for recording function in scaler 0)
    Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);
    Start_Video_Camera();

#if RAM_TABLE_DYNAMIC_LOADING
    // Copy to RAM to increase the speed of dynamic loading.
    memcpy(ram_drp_lib_bayer2grayscale, g_drp_lib_bayer2grayscale, sizeof(ram_drp_lib_bayer2grayscale));
    memcpy(ram_drp_lib_median_blur, g_drp_lib_median_blur, sizeof(ram_drp_lib_median_blur));
    memcpy(ram_drp_lib_canny_calculate, g_drp_lib_canny_calculate, sizeof(ram_drp_lib_canny_calculate));
    memcpy(ram_drp_lib_canny_hysterisis, g_drp_lib_canny_hysterisis, sizeof(ram_drp_lib_canny_hysterisis));
#endif

    R_DK2_Initialize();

    t.start();

    while (true) {
        ThisThread::flags_wait_all(DRP_FLG_CAMER_IN);

        /* Load DRP Library            */
        /*        +------------------+ */
        /* tile 0 | Bayer2Grayscale  | */
        /*        +------------------+ */
        /* tile 1 | Bayer2Grayscale  | */
        /*        +------------------+ */
        /* tile 2 | Bayer2Grayscale  | */
        /*        +------------------+ */
        /* tile 3 | Bayer2Grayscale  | */
        /*        +------------------+ */
        /* tile 4 | Bayer2Grayscale  | */
        /*        +------------------+ */
        /* tile 5 | Bayer2Grayscale  | */
        /*        +------------------+ */
        /* fbuf_bayer -> fbuf_work0    */
        t.reset();
        R_DK2_Load(
#if RAM_TABLE_DYNAMIC_LOADING
                   ram_drp_lib_bayer2grayscale,
#else
                   g_drp_lib_bayer2grayscale,
#endif
                   R_DK2_TILE_0 | R_DK2_TILE_1 | R_DK2_TILE_2 | R_DK2_TILE_3 | R_DK2_TILE_4 | R_DK2_TILE_5,
                   R_DK2_TILE_PATTERN_1_1_1_1_1_1, NULL, &cb_drp_finish, drp_lib_id);
        R_DK2_Activate(0, 0);
        drp_time[0] = t.read_us();

        t.reset();
        r_drp_bayer2grayscale_t * param_b2g = (r_drp_bayer2grayscale_t *)nc_memory;
        for (idx = 0; idx < R_DK2_TILE_NUM; idx++) {
            param_b2g[idx].src    = (uint32_t)fbuf_bayer + (VIDEO_PIXEL_HW * (VIDEO_PIXEL_VW / R_DK2_TILE_NUM) * idx);
            param_b2g[idx].dst    = (uint32_t)fbuf_work0 + (VIDEO_PIXEL_HW * (VIDEO_PIXEL_VW / R_DK2_TILE_NUM) * idx);
            param_b2g[idx].width  = VIDEO_PIXEL_HW;
            param_b2g[idx].height = VIDEO_PIXEL_VW / R_DK2_TILE_NUM;
            param_b2g[idx].top    = (idx == 0) ? 1 : 0;
            param_b2g[idx].bottom = (idx == 5) ? 1 : 0;
            R_DK2_Start(drp_lib_id[idx], (void *)&param_b2g[idx], sizeof(r_drp_bayer2grayscale_t));
        }
        ThisThread::flags_wait_all(DRP_FLG_TILE_ALL);
        R_DK2_Unload(0, drp_lib_id);
        drp_time[1] = t.read_us();


        /* Load DRP Library            */
        /*        +------------------+ */
        /* tile 0 | MedianBlur       | */
        /*        +------------------+ */
        /* tile 1 | MedianBlur       | */
        /*        +------------------+ */
        /* tile 2 | MedianBlur       | */
        /*        +------------------+ */
        /* tile 3 | MedianBlur       | */
        /*        +------------------+ */
        /* tile 4 | MedianBlur       | */
        /*        +------------------+ */
        /* tile 5 | MedianBlur       | */
        /*        +------------------+ */
        /* fbuf_work0 -> fbuf_work1    */
        t.reset();
        R_DK2_Load(
#if RAM_TABLE_DYNAMIC_LOADING
                   ram_drp_lib_median_blur,
#else
                   g_drp_lib_median_blur,
#endif
                   R_DK2_TILE_0 | R_DK2_TILE_1 | R_DK2_TILE_2 | R_DK2_TILE_3 | R_DK2_TILE_4 | R_DK2_TILE_5,
                   R_DK2_TILE_PATTERN_1_1_1_1_1_1, NULL, &cb_drp_finish, drp_lib_id);
        R_DK2_Activate(0, 0);
        drp_time[2] = t.read_us();

        t.reset();
        r_drp_median_blur_t * param_median = (r_drp_median_blur_t *)nc_memory;
        for (idx = 0; idx < R_DK2_TILE_NUM; idx++) {
            param_median[idx].src    = (uint32_t)fbuf_work0 + (VIDEO_PIXEL_HW * (VIDEO_PIXEL_VW / R_DK2_TILE_NUM) * idx);
            param_median[idx].dst    = (uint32_t)fbuf_work1 + (VIDEO_PIXEL_HW * (VIDEO_PIXEL_VW / R_DK2_TILE_NUM) * idx);
            param_median[idx].width  = VIDEO_PIXEL_HW;
            param_median[idx].height = VIDEO_PIXEL_VW / R_DK2_TILE_NUM;
            param_median[idx].top    = (idx == 0) ? 1 : 0;
            param_median[idx].bottom = (idx == 5) ? 1 : 0;
            R_DK2_Start(drp_lib_id[idx], (void *)&param_median[idx], sizeof(r_drp_median_blur_t));
        }
        ThisThread::flags_wait_all(DRP_FLG_TILE_ALL);
        R_DK2_Unload(0, drp_lib_id);
        drp_time[3] = t.read_us();


        /* Load DRP Library            */
        /*        +------------------+ */
        /* tile 0 |                  | */
        /*        + CannyCalculate   + */
        /* tile 1 |                  | */
        /*        +------------------+ */
        /* tile 2 |                  | */
        /*        + CannyCalculate   + */
        /* tile 3 |                  | */
        /*        +------------------+ */
        /* tile 4 |                  | */
        /*        + CannyCalculate   + */
        /* tile 5 |                  | */
        /*        +------------------+ */
        /* fbuf_work1 -> fbuf_work0    */
        t.reset();
        R_DK2_Load(
#if RAM_TABLE_DYNAMIC_LOADING
                   ram_drp_lib_canny_calculate,
#else
                   g_drp_lib_canny_calculate,
#endif
                   R_DK2_TILE_0 | R_DK2_TILE_2 | R_DK2_TILE_4,
                   R_DK2_TILE_PATTERN_2_2_2, NULL, &cb_drp_finish, drp_lib_id);
        R_DK2_Activate(0, 0);
        drp_time[4] = t.read_us();

        t.reset();
        r_drp_canny_calculate_t * param_canny_cal = (r_drp_canny_calculate_t *)nc_memory;
        for (idx = 0; idx < 3; idx++) {
            param_canny_cal[idx].src    = (uint32_t)fbuf_work1 + (VIDEO_PIXEL_HW * (VIDEO_PIXEL_VW / 3) * idx);
            param_canny_cal[idx].dst    = (uint32_t)fbuf_work0 + (VIDEO_PIXEL_HW * (VIDEO_PIXEL_VW / 3) * idx);
            param_canny_cal[idx].width  = VIDEO_PIXEL_HW;
            param_canny_cal[idx].height = (VIDEO_PIXEL_VW / 3);
            param_canny_cal[idx].top    = ((idx * 2) == 0) ? 1 : 0;
            param_canny_cal[idx].bottom = ((idx * 2) == 4) ? 1 : 0;
            param_canny_cal[idx].work   = (uint32_t)&drp_work_buf[((VIDEO_PIXEL_HW * ((VIDEO_PIXEL_VW / 3) + 2)) * 2) * idx];
            param_canny_cal[idx].threshold_high = 0x28;
            param_canny_cal[idx].threshold_low  = 0x18;
            R_DK2_Start(drp_lib_id[(idx * 2)], (void *)&param_canny_cal[idx], sizeof(r_drp_canny_calculate_t));
        }
        ThisThread::flags_wait_all(DRP_FLG_TILE_ALL);
        R_DK2_Unload(0, drp_lib_id);
        drp_time[5] = t.read_us();


        /* Load DRP Library            */
        /*        +------------------+ */
        /* tile 0 |                  | */
        /*        +                  + */
        /* tile 1 |                  | */
        /*        +                  + */
        /* tile 2 |                  | */
        /*        + CannyHysterisis  + */
        /* tile 3 |                  | */
        /*        +                  + */
        /* tile 4 |                  | */
        /*        +                  + */
        /* tile 5 |                  | */
        /*        +------------------+ */
        /* fbuf_work0 -> fbuf_clat8    */
        t.reset();
        R_DK2_Load(
#if RAM_TABLE_DYNAMIC_LOADING
                   ram_drp_lib_canny_hysterisis,
#else
                   g_drp_lib_canny_hysterisis,
#endif
                   R_DK2_TILE_0,
                   R_DK2_TILE_PATTERN_6, NULL, &cb_drp_finish, drp_lib_id);
        R_DK2_Activate(0, 0);
        drp_time[6] = t.read_us();

        t.reset();
        r_drp_canny_hysterisis_t * param_canny_hyst = (r_drp_canny_hysterisis_t *)nc_memory;
        param_canny_hyst[0].src    = (uint32_t)fbuf_work0;
        param_canny_hyst[0].dst    = (uint32_t)fbuf_clat8;
        param_canny_hyst[0].width  = VIDEO_PIXEL_HW;
        param_canny_hyst[0].height = VIDEO_PIXEL_VW;
        param_canny_hyst[0].work   = (uint32_t)drp_work_buf;
        param_canny_hyst[0].iterations = 2;
        R_DK2_Start(drp_lib_id[0], (void *)&param_canny_hyst[0], sizeof(r_drp_canny_hysterisis_t));
        ThisThread::flags_wait_all(DRP_FLG_TILE_ALL);
        R_DK2_Unload(0, drp_lib_id);
        drp_time[7] = t.read_us();


        // Display DRP time
        sprintf(str, "Bayer2Grayscale : Load %2.1fms + Run %2.1fms",
                (float_t)drp_time[0] / 1000, (float_t)drp_time[1] / 1000);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 0, 1, 2);
        sprintf(str, "MedianBlur      : Load %2.1fms + Run %2.1fms",
                (float_t)drp_time[2] / 1000, (float_t)drp_time[3] / 1000);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 2, 1, 2);
        sprintf(str, "CannyCalculate  : Load %2.1fms + Run %2.1fms",
                (float_t)drp_time[4] / 1000, (float_t)drp_time[5] / 1000);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 4, 1, 2);
        sprintf(str, "CannyHysterisis : Load %2.1fms + Run %2.1fms",
                (float_t)drp_time[6] / 1000, (float_t)drp_time[7] / 1000);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 6, 1, 2);

        uint32_t time_sum = 0;
        for (int i = 0; i < 8; i++) {
            time_sum += drp_time[i];
        }
        sprintf(str, "Total           : %2.1fms", (float_t)time_sum / 1000);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 8, 1, 2);
    }
}

int main(void) {
    // Start DRP task
    drpTask.start(callback(drp_task));

    wait(osWaitForever);
}

#endif
