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

#if (SAMPLE_PROGRAM_NO == 17)
// SAMPLE_PROGRAM_NO 17 : USBMSD and FlashAPI sample advanced version
//
// USBMSD and FlashAPI sample advanced version
// It is a sample program that can read and write serial flash from the PC via Filesystem.
// By connecting the board and the PC with a USB cable, you can refer inside the device as a mass storage device.
// (The format is required when connecting for the first time.)
// When you write a JPEG file (.jpg) from the PC to the storage, the image is displayed.
// By pressing SW3, change the save destination to heap memory.
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBMSD.h"
#include "ObservingBlockDevice.h"
#include "FlashIAPBlockDevice.h"
#include "mbed_drv_cfg.h"
#include "HeapBlockDevice.h"
#include "dcache-control.h"
#include "EasyAttach_CameraAndLCD.h"
#include "FATFileSystem.h"
#include "JPEG_Converter.h"

#define FILE_NAME_LEN          (64)
#define FRAME_BUFFER_STRIDE    (((LCD_PIXEL_WIDTH * 2) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (LCD_PIXEL_HEIGHT)

static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t JpegBuffer[1024 * 128]__attribute((aligned(32)));
DisplayBase Display;

typedef enum {
    BD_FLASHIAP,
    BD_HEAP,
    BD_NUM
} bd_type_t;

static FlashIAPBlockDevice flashiap_bd(FLASH_BASE + 0x100000, FLASH_SIZE - 0x100000);
static HeapBlockDevice heap_bd(0x100000);

static BlockDevice * base_bd[BD_NUM] = {
    &flashiap_bd,    // BD_FLASHIAP
    &heap_bd         // BD_HEAP
};

static const char * base_bd_str[BD_NUM] = {
    "FlashIAP",      // BD_FLASHIAP
    "Heap"           // BD_HEAP
};

static InterruptIn storage_btn(USER_BUTTON0);
static int tmp_bd_index = 0;
static int bd_index = -1;
static bool storage_change_flg = false;
static Timer storage_change_timer;
static FATFileSystem * p_fs = NULL;
static ObservingBlockDevice * p_observing_bd = NULL;
static USBMSD * p_usb = NULL;
static Thread msdTask(osPriorityAboveNormal);
static Semaphore usb_sem(0);
static bool heap_bd_format = false;
static bool image_disp = false;

// Function prototypes
static void msd_task(void);
static void storage_change(BlockDevice * p_bd);
static void chk_storage_change(void);
static void chk_bd_change(void);
static void storage_btn_fall(void);
static void clear_display(void);
static void Start_LCD_Display(void);

static void usb_callback_func(void) {
    usb_sem.release();
}

static void msd_task(void) {
    while (true) {
        usb_sem.wait();
        if (p_usb != NULL) {
            p_usb->process();
        }
    }
}

static void storage_change(BlockDevice * p_bd) {
    storage_change_flg = true;
    storage_change_timer.reset();
    storage_change_timer.start();
    if (image_disp) {
        clear_display();
        image_disp = false;
    }
}

static void chk_storage_change(void) {
    if (!storage_change_flg) {
        return;
    }

    // wait storage change
    while (storage_change_timer.read_ms() < 1000) {
        ThisThread::sleep_for(100);
    }
    storage_change_timer.stop();
    storage_change_timer.reset();
    storage_change_flg = false;

    p_fs->unmount();
    chk_bd_change();
    p_fs->mount(base_bd[bd_index]);
}

static void chk_bd_change(void) {
    if (bd_index == tmp_bd_index) {
        return;
    }
    if (p_usb != NULL) {
        USBMSD * wk = p_usb;
        p_usb = NULL;
        delete wk;
    }
    if (p_observing_bd != NULL) {
        delete p_observing_bd;
    }
    if (p_fs != NULL) {
        delete p_fs;
    }
    bd_index = tmp_bd_index;
    printf("\r\nconnecting %s\r\n", base_bd_str[bd_index]);
    if (bd_index == BD_HEAP) {
        if (!heap_bd_format) {
            FATFileSystem::format(&heap_bd);
            heap_bd_format = true;
        }
    }
    p_fs = new FATFileSystem(base_bd_str[bd_index]);
    p_observing_bd = new ObservingBlockDevice(base_bd[bd_index]);
    p_observing_bd->attach(&storage_change);
    p_usb = new USBMSD(p_observing_bd);
    p_usb->attach(&usb_callback_func);
}

static void storage_btn_fall(void) {
    if ((tmp_bd_index + 1) < BD_NUM) {
        tmp_bd_index++;
    } else {
        tmp_bd_index = 0;
    }
    storage_change(NULL);
}

static void clear_display(void) {
    // Initialize the background to black
    for (uint32_t i = 0; i < sizeof(user_frame_buffer0); i += 2) {
        user_frame_buffer0[i + 0] = 0x00;
        user_frame_buffer0[i + 1] = 0x80;
    }
    dcache_clean(user_frame_buffer0, sizeof(user_frame_buffer0));
}

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;

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
    char file_path[10 + FILE_NAME_LEN];
    int file_num;
    JPEG_Converter  Jcu;
    long file_size;

    clear_display();
    EasyAttach_Init(Display);
    Start_LCD_Display();

    // Start usb task
    msdTask.start(&msd_task);

    // Set BlockDevice
    chk_bd_change();
    p_fs->mount(base_bd[bd_index]);

    // Button setting
    storage_btn.fall(&storage_btn_fall);

    while (true) {
        // Confirm storage change
        chk_storage_change();

        // File search
        file_num = 0;
        sprintf(file_path, "/%s/", base_bd_str[bd_index]);
        d = opendir(file_path);
        if (d != NULL) {
            while ((p = readdir(d)) != NULL) {
                size_t len = strlen(p->d_name);
                if (len < FILE_NAME_LEN) {
                    // Make file path
                    sprintf(file_path, "/%s/%s", base_bd_str[bd_index], p->d_name);
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
                                    image_disp = true;
                                    file_num++;
                                } else {
                                    printf("Decode error.\r\n");
                                }
                            } else {
                                printf("File size over.\r\n");
                            }
                            fclose(fp);

                            for (int i = 0; (i < 50) && (storage_change_flg == false); i++) {
                                ThisThread::sleep_for(100);
                            }
                        } else {
                            printf("File open error.\r\n");
                        }
                    }
                }
                if (storage_change_flg) {
                    clear_display();
                    break;
                }
            }
            closedir(d);
        }

        // If there is no files, wait until the storage is changed
        if ((file_num == 0) && (!storage_change_flg)) {
            printf("No files\r\n");
            while (!storage_change_flg) {
                ThisThread::sleep_for(100);
            }
        }
    }
}

#endif
