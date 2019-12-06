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

#if (SAMPLE_PROGRAM_NO == 4)
// SAMPLE_PROGRAM_NO  4 : SSIF wav playback sample (use USB memory or SD card)
//
// This sample will play a ".wav" file of the SD root folder.
// Wav file format : RIFF format, 1ch or 2ch, 44.1kHz, 16bits

#if defined(TARGET_SEMB1402) || defined(TARGET_RZ_A2M_SBEV)
#error "Audio is not supported."
#endif

#include "mbed.h"
#include "FATFileSystem.h"
#include "SdUsbConnect.h"
#include "EasyPlayback.h"
#include "EasyDec_WavCnv2ch.h"

static EasyPlayback AudioPlayer;

#define FILE_NAME_LEN          (64)
#define MOUNT_NAME             "storage"

static InterruptIn skip_btn(USER_BUTTON0);

static void skip_btn_fall(void) {
    AudioPlayer.skip();
}

int main() {
    DIR  * d;
    struct dirent * p;
    char file_path[sizeof("/" MOUNT_NAME "/") + FILE_NAME_LEN];
    SdUsbConnect storage(MOUNT_NAME);

    // decoder setting
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".wav");
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".WAV");

    AudioPlayer.outputVolume(0.5);

    // button setting
    skip_btn.fall(&skip_btn_fall);

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

                    // playback
                    AudioPlayer.play(file_path);
                }
            }
            closedir(d);
        }
    }
}

#endif
