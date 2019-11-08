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

#if (SAMPLE_PROGRAM_NO == 0)
// SAMPLE_PROGRAM_NO  0 : DigitalOut, InterruptIn, RTC, Timer and AnalogIn sample
//
// LED1 will blink at 1 second intervals.
// While pressing USER_BUTTON 0, LED2 lights up.
// RTC value, Timer value and AD value are displayed every 1 second.

#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
InterruptIn button(USER_BUTTON0);
#if defined(TARGET_RZ_A2M_EVB)
AnalogIn adin(P5_6); // In TARGET_RZ_A2M_EVB, SW5 and SW4 are connected to P5_6 as AD switches.
#else
AnalogIn adin(A0);   // For other Mbed boards
#endif

static void button_fall(void) {
    led2 = !led2;
}

static void button_rise(void) {
    led2 = 0;
}

int main(void) {
    // InterruptIn
    button.fall(&button_fall);
    button.rise(&button_rise);

    // RTC
    set_time(0);

    // Timer
    Timer tick_timer;
    tick_timer.reset();
    tick_timer.start();

    while (1) {
        // DigitalOut
        led1 = !led1;

        // RTC
        time_t seconds = time(NULL); // UTC
        struct tm *t = localtime(&seconds);
        printf("RTC:%02d/%02d/%04d %02d:%02d:%02d\r\n",
          t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);

        // Timer
        printf("Timer:%dms\r\n", tick_timer.read_ms());

        // AnalogIn
        printf("A/D:%f\r\n", (float)adin);

        // OS wait 1 second
        ThisThread::sleep_for(1000);
    }
}

#endif
