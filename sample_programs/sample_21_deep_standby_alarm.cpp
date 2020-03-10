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

#if (SAMPLE_PROGRAM_NO == 21)
// SAMPLE_PROGRAM_NO 21 : Deep standby and RTC alarm sample
//
// Wake up from deep standby every 5 seconds using RTC alarm interrupt.
// It also wakes up when BUTTON1(SW2) is pressed.

#include "mbed.h"
#include "AlarmTimer.h"
#include "DeepStandby.h"

// On-Chip Data Retention RAM
#if defined ( __CC_ARM ) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#define DATA_RETENTION_RAM ".bss.NoInit"
#else
#define DATA_RETENTION_RAM "NV_DATA"
#endif
static int wake_up_cnt __attribute((section(DATA_RETENTION_RAM)));

int main() {
    time_t seconds;
    DeepStandby::cancel_src_simple_t cancel_src;

    if (DeepStandby::GetCancelSourceSimple(&cancel_src) == false) {
        // Reset start
        printf("Reset start\r\n");
        // initialization of On-Chip Data Retention RAM
        wake_up_cnt = 0;
    } else {
        // Deep standby cancel
        wake_up_cnt++;
        printf("Deep standby cancel %d :", wake_up_cnt);
        if (cancel_src.button0) {  // Works with RZ_A2M_EVB and SEMB1402.
            printf(" BUTTON0");
        }
        if (cancel_src.button1) {  // Works with RZ_A2M_EVB and RZ_A2M_SBEV.
            printf(" BUTTON1");
        }
        if (cancel_src.rtc) {      // Works with RZ_A1H, RZ_A2M_EVB, RZ_A2M_SBEV and SEMB1402.
            printf(" RTC");
        }
        printf("\r\n");
    }

    // RTC time
    seconds = time(NULL);
    struct tm *t = localtime(&seconds);
    printf("RTC time : %02d/%02d/%04d %02d:%02d:%02d\r\n\r\n",
           t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);

    // Set alarm interrupt
    AlarmTimer::set(seconds + 5);  // 5 seconds later

    // Set deep standby mode
    cancel_src.button0 = false;    // Works with RZ_A2M_EVB and SEMB1402. (Used with custom boot loader)
    cancel_src.button1 = true;     // Works with RZ_A2M_EVB and RZ_A2M_SBEV.
    cancel_src.rtc     = true;     // Works with RZ_A1H, RZ_A2M_EVB, RZ_A2M_SBEV and SEMB1402.
    DeepStandby::SetDeepStandbySimple(&cancel_src);
}

#endif
