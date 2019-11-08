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

#if (SAMPLE_PROGRAM_NO == 6)
// SAMPLE_PROGRAM_NO  6 : LCD, Touch panel and JCU sample (use USB memory or SD card)
//
// Decodes the JPEG file in the USB memory and displays it on the LED.
// Touch information displays on the JPEG image.
// Up to 2 touch positions can be recognized at the same time.

#if defined(TARGET_SEMB1402) || defined(TARGET_RZ_A2M_SBEV)
#error "LCD is not supported."
#endif

#include "mbed.h"
#include "FATFileSystem.h"
#include "SdUsbConnect.h"
#include "dcache-control.h"
#include "EasyAttach_CameraAndLCD.h"
#include "JPEG_Converter.h"

#define FRAME_BUFFER_STRIDE    (((LCD_PIXEL_WIDTH * 2) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (LCD_PIXEL_HEIGHT)

#define FILE_NAME_LEN          (64)
#define MOUNT_NAME             "storage"

static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t JpegBuffer[128 * 1024]__attribute((aligned(32)));

DisplayBase Display;


#if defined(TouckKey_LCD_shield)

/* TOUCH BUFFER Parameter GRAPHICS_LAYER_2 */
#define TOUCH_BUFFER_BYTE_PER_PIXEL   (2u)
#define TOUCH_BUFFER_STRIDE           (((LCD_PIXEL_WIDTH * TOUCH_BUFFER_BYTE_PER_PIXEL) + 31u) & ~31u)

/* Touch panel parameter */
#define TOUCH_NUM                     (2u)
#define DRAW_POINT                    (8)

static Semaphore   sem_touch_int(0);
static uint8_t user_frame_buffer_touch[TOUCH_BUFFER_STRIDE * LCD_PIXEL_HEIGHT]__attribute((aligned(32)));

static void draw_touch_pos(uint8_t * p_buf, int id, int x, int y) {
    int idx_base;
    int wk_idx;
    int i;
    int j;
    uint8_t coller_pix[TOUCH_BUFFER_BYTE_PER_PIXEL];  /* ARGB4444 */

    /* A coordinate in the upper left is calculated from a central coordinate. */
    if ((x - (DRAW_POINT / 2)) >= 0) {
        x -= (DRAW_POINT / 2);
    }
    if (x > ((int)LCD_PIXEL_WIDTH - DRAW_POINT)) {
        x = ((int)LCD_PIXEL_WIDTH - DRAW_POINT);
    }
    if ((y - (DRAW_POINT / 2)) >= 0) {
        y -= (DRAW_POINT / 2);
    }
    if (y > ((int)LCD_PIXEL_HEIGHT - DRAW_POINT)) {
        y = ((int)LCD_PIXEL_HEIGHT - DRAW_POINT);
    }
    idx_base = (x + ((int)LCD_PIXEL_WIDTH * y)) * TOUCH_BUFFER_BYTE_PER_PIXEL;

    /* Select color */
    if (id == 0) {
        /* Blue */
        coller_pix[0] = 0x0F;  /* 4:Green 4:Blue */
        coller_pix[1] = 0xF0;  /* 4:Alpha 4:Red  */
    } else {
        /* Pink */
        coller_pix[0] = 0x07;  /* 4:Green 4:Blue */
        coller_pix[1] = 0xFF;  /* 4:Alpha 4:Red  */
    }

    /* Drawing */
    for (i = 0; i < DRAW_POINT; i++) {
        wk_idx = idx_base + ((int)LCD_PIXEL_WIDTH * TOUCH_BUFFER_BYTE_PER_PIXEL * i);
        for (j = 0; j < DRAW_POINT; j++) {
            p_buf[wk_idx++] = coller_pix[0];
            p_buf[wk_idx++] = coller_pix[1];
        }
    }
}

static void touch_int_callback(void) {
    sem_touch_int.release();
}

static void touch_task(void) {
    DisplayBase::rect_t rect;
    TouchKey::touch_pos_t touch_pos[TOUCH_NUM];
    int touch_num = 0;
    int touch_num_last = 0;
    uint32_t i;
#if defined(TARGET_RZ_A1XX)
    TouckKey_LCD_shield touch(P4_0, P2_13, I2C_SDA, I2C_SCL);
#elif defined(TARGET_RZ_A2XX)
    TouckKey_LCD_shield touch(NC, P5_7, I2C_SDA, I2C_SCL);
#endif

    /* The layer by which the touch panel location is drawn */
    memset(user_frame_buffer_touch, 0, sizeof(user_frame_buffer_touch));
    dcache_clean(user_frame_buffer_touch, sizeof(user_frame_buffer_touch));
    rect.vs = 0;
    rect.vw = LCD_PIXEL_HEIGHT;
    rect.hs = 0;
    rect.hw = LCD_PIXEL_WIDTH;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_2,
        (void *)user_frame_buffer_touch,
        TOUCH_BUFFER_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_ARGB4444,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_2);

    /* Callback setting */
    touch.SetCallback(&touch_int_callback);

    /* Reset touch IC */
    touch.Reset();

    while (1) {
        /* Wait touch event */
        sem_touch_int.wait();

        /* Get touch coordinates */
        touch_num = touch.GetCoordinates(TOUCH_NUM, touch_pos);

        /* When it's a new touch, touch frame buffer is initialized */
        if ((touch_num != 0) && (touch_num_last == 0)) {
            memset(user_frame_buffer_touch, 0, sizeof(user_frame_buffer_touch));
        }
        touch_num_last = touch_num;

        /* Drawing of a touch coordinate */
        for (i = 0; i < TOUCH_NUM; i ++) {
            printf("{valid=%d,x=%lu,y=%lu} ", touch_pos[i].valid, touch_pos[i].x, touch_pos[i].y);
            if (touch_pos[i].valid) {
                draw_touch_pos(user_frame_buffer_touch, i, touch_pos[i].x, touch_pos[i].y);
            }
        }
        printf("\r\n");

        /* Data cache clean */
        dcache_clean(user_frame_buffer_touch, sizeof(user_frame_buffer_touch));
    }
}
#endif // TouckKey_LCD_shield

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;

    // Initialize the background to black
    for (uint32_t i = 0; i < sizeof(user_frame_buffer0); i += 2) {
        user_frame_buffer0[i + 0] = 0x00;
        user_frame_buffer0[i + 1] = 0x80;
    }
    dcache_clean(user_frame_buffer0, sizeof(user_frame_buffer0));

    rect.vs = 0;
    rect.vw = LCD_PIXEL_HEIGHT;
    rect.hs = 0;
    rect.hw = LCD_PIXEL_WIDTH;
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

int main() {
    FILE * fp;
    DIR  * d;
    struct dirent * p;
    char file_path[sizeof("/" MOUNT_NAME "/") + FILE_NAME_LEN];
    SdUsbConnect storage(MOUNT_NAME);
    JPEG_Converter  Jcu;
    long file_size;

    EasyAttach_Init(Display);
    Start_LCD_Display();

    /* Start touch panel processing */
#if defined(TouckKey_LCD_shield)
    Thread touchTask;
    touchTask.start(callback(touch_task));
#endif // TouckKey_LCD_shield

    while (1) {
        // connect wait
        storage.wait_connect();

        // file search
        d = opendir("/" MOUNT_NAME "/");
        if (d != NULL) {
            while ((p = readdir(d)) != NULL) {
                size_t len = strlen(p->d_name);
                if (len < FILE_NAME_LEN) {
                    // make file path
                    sprintf(file_path, "/%s/%s", MOUNT_NAME, p->d_name);
                    printf("%s\r\n", file_path);

                    char *extension = strstr(file_path, ".");
                    if ((extension != NULL) && (memcmp(extension, ".jpg", 4) == 0)) {
                        fp = fopen(file_path, "rb");
                        if (fp != NULL) {
                            fseek(fp, 0, SEEK_END);
                            file_size = ftell(fp);
                            fseek(fp, 0, SEEK_SET);
                            if (file_size <= (long)sizeof(JpegBuffer)) {
                                JPEG_Converter::bitmap_buff_info_t bitmap_buff_info;
                                JPEG_Converter::decode_options_t   decode_options;

                                bitmap_buff_info.width              = LCD_PIXEL_WIDTH;
                                bitmap_buff_info.height             = LCD_PIXEL_HEIGHT;
                                bitmap_buff_info.format             = JPEG_Converter::WR_RD_YCbCr422;
                                bitmap_buff_info.buffer_address     = (void *)user_frame_buffer0;

                                decode_options.output_swapsetting   = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;

                                setvbuf(fp, NULL, _IONBF, 0); // unbuffered
                                fread(JpegBuffer, sizeof(char), file_size, fp);
                                dcache_clean(JpegBuffer, file_size);

                                if (Jcu.decode((void *)JpegBuffer, &bitmap_buff_info, &decode_options) == JPEG_Converter::JPEG_CONV_OK) {
                                    printf("Decode ok.\r\n");
                                } else {
                                    printf("Decode error.\r\n");
                                }
                            } else {
                                printf("File size over.\r\n");
                            }
                            fclose(fp);

                            ThisThread::sleep_for(5000);
                        } else {
                            printf("File open error.\r\n");
                        }
                    } else {
                        printf("File skip.\r\n");
                    }
                }
            }
            closedir(d);
        }
    }
}

#endif
