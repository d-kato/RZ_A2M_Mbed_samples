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

#if (SAMPLE_PROGRAM_NO == 3)
// SAMPLE_PROGRAM_NO  3 : SPDIF loop back sample
//
// The input sound is output as it is.
// In case of SAMPLE_PROGRAM_NO=3 :
//   Since TARGET_RZ_A2M_EVB has no connector and no suitable AUDIO_CLK has been entered, only pseudo-evaluation is executed.
//   - Please connect PC_5 and PC_4 when evaluating.
//   - Since AUDIO_CLK on the board is 11.2896 MHz, it operates at half the actual speed.
//   - The AUDIO_CLK frequency required is 512 times as large as the sample frequency for audio data.
//     (fs=44.1kHz : AUDIO_CLK=22.5792MHz, fs=48kHz : AUDIO_CLK=24.5760MHz)

#if !defined(TARGET_RZ_A2M_EVB)
#error "SPDIF is not supported."
#endif

#include "mbed.h"
#include "SPDIF_RBSP.h"

#define WRITE_BUFF_NUM         (8)
#define READ_BUFF_NUM          (8)
#define MAIL_QUEUE_SIZE        (WRITE_BUFF_NUM + READ_BUFF_NUM + 1)
#define INFO_TYPE_WRITE_END    (0)
#define INFO_TYPE_READ_END     (1)

#define AUDIO_BUFF_SIZE        (192 * 2 * 10) // 1 block * 2ch * 10
SPDIF_RBSP audio(P6_4, PC_5, PC_4, false, 0x80, WRITE_BUFF_NUM, READ_BUFF_NUM);

typedef struct {
    uint32_t info_type;
    void *   p_data;
    int32_t  result;
} mail_t;
Mail<mail_t, MAIL_QUEUE_SIZE> mail_box;

//32 bytes aligned! No cache memory
static uint8_t audio_read_buff[READ_BUFF_NUM][AUDIO_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));

static void callback_audio(void * p_data, int32_t result, void * p_app_data) {
    mail_t *mail = mail_box.alloc();

    if (mail == NULL) {
        printf("error mail alloc\r\n");
    } else {
        mail->info_type = (uint32_t)p_app_data;
        mail->p_data    = p_data;
        mail->result    = result;
        mail_box.put(mail);
    }
}

int main() {
    rbsp_data_conf_t audio_write_conf = {&callback_audio, (void *)INFO_TYPE_WRITE_END};
    rbsp_data_conf_t audio_read_conf  = {&callback_audio, (void *)INFO_TYPE_READ_END};

    // Read buffer setting
    for (int i = 0; i < READ_BUFF_NUM; i++) {
        if (audio.read(audio_read_buff[i], AUDIO_BUFF_SIZE, &audio_read_conf) < 0) {
            printf("read error\r\n");
        }
    }

    while (1) {
        osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t *)evt.value.p;

            if ((mail->info_type == INFO_TYPE_READ_END) && (mail->result > 0)) {
                audio.write(mail->p_data, mail->result, &audio_write_conf);
            } else {
                audio.read(mail->p_data, AUDIO_BUFF_SIZE, &audio_read_conf); // Resetting read buffer
            }
            mail_box.free(mail);
        }
    }
}

#endif
