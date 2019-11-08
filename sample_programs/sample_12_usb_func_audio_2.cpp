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

#if (SAMPLE_PROGRAM_NO == 12)
// SAMPLE_PROGRAM_NO 12 : USBAudio and SSIF sample
// Sound from PC will be output to SSIF. (speaker)
// Sound from SSIF will be input to the PC. (microphone)
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#if defined(TARGET_SEMB1402) || defined(TARGET_RZ_A2M_SBEV)
#error "Audio is not supported."
#endif

#include "mbed.h"
#include "USBAudio.h"
#include "AUDIO_GRBoard.h"

#define AUDIO_OUT_BUFF_NUM     (8)
#define AUDIO_IN_BUFF_NUM      (8)
#define AUDIO_BUFF_SIZE        (4096)
AUDIO_GRBoard i2s_audio(0x80, AUDIO_OUT_BUFF_NUM - 1, AUDIO_IN_BUFF_NUM);

//32 bytes aligned! No cache memory
static uint8_t audio_out_buff[AUDIO_OUT_BUFF_NUM][AUDIO_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));
static uint8_t audio_in_buff[AUDIO_IN_BUFF_NUM][AUDIO_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));

Semaphore   mic_in_sem(0);

static void callback_audio_in(void * p_data, int32_t result, void * p_app_data) {
    mic_in_sem.release();
}

void mic_task(USBAudio * p_usb_audio) {
    rbsp_data_conf_t audio_in_cb = {&callback_audio_in, NULL};
    uint8_t * p_buf;
    int audio_in_idx = 0;

    // Read buffer setting
    for (int i = 0; i < AUDIO_IN_BUFF_NUM; i++) {
        i2s_audio.read(audio_in_buff[i], AUDIO_BUFF_SIZE, &audio_in_cb);
    }

    while (1) {
        mic_in_sem.wait();
        p_buf = audio_in_buff[audio_in_idx++];
        if (audio_in_idx >= AUDIO_IN_BUFF_NUM) {
            audio_in_idx = 0;
        }
        if (i2s_audio.read(p_buf, AUDIO_BUFF_SIZE, &audio_in_cb) < 0) {
            memset(p_buf, 0, AUDIO_BUFF_SIZE);
        }
        p_usb_audio->write(p_buf, AUDIO_BUFF_SIZE);
    }
}

int main() {
    USBAudio usb_audio(true, 44100, 2, 44100, 2);
    rbsp_data_conf_t audio_out_cb = {NULL, NULL};
    uint8_t * p_buf;
    int audio_out_idx = 0;

    i2s_audio.power();                // For GR-PEACH and GR-LYCHEE
    i2s_audio.outputVolume(0.5, 0.5); // For GR-PEACH and GR-LYCHEE
    i2s_audio.micVolume(0.7);         // For GR-PEACH and GR-LYCHEE

    Thread micTask;
    micTask.start(callback(mic_task, &usb_audio));

    while (1) {
        p_buf = audio_out_buff[audio_out_idx++];
        if (audio_out_idx >= AUDIO_OUT_BUFF_NUM) {
            audio_out_idx = 0;
        }
        if (!usb_audio.read(p_buf, AUDIO_BUFF_SIZE)) {
            memset(p_buf, 0, AUDIO_BUFF_SIZE);
        }
        i2s_audio.write(p_buf, AUDIO_BUFF_SIZE, &audio_out_cb);
    }
}

#endif
