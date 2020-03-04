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
#ifndef SAMPLE_SELECT_H
#define SAMPLE_SELECT_H

// You can try each sample program by changing the following macro.
#define SAMPLE_PROGRAM_NO  0

// No.  Program file                   Description
//
//  0 : sample_00_led_rtc_analogin     DigitalOut, InterruptIn, RTC, Timer and AnalogIn sample
//  1 : sample_01_flash_write          FlashAPI sample
//  2 : sample_02_ssif_loop_back       SSIF loop back sample
//  3 : sample_03_spdif_loop_back      SPDIF loop back sample
//  4 : sample_04_ssif_wav_playback    SSIF wav playback sample (use USB memory or SD card)
//  5 : sample_05_spdif_wav_playback   SPDIF wav playback sample (use USB memory or SD card)
//  6 : sample_06_lcd_touch_jcu        LCD, Touch panel and JCU sample (use USB memory or SD card)
//  7 : sample_07_usb_func_serial      USBSerial (CDC) sample
//  8 : sample_08_usb_func_mouse       USBMouse sample
//  9 : sample_09_usb_func_keyboard    USBKeyboard sample
// 10 : sample_10_usb_func_midi        USBMIDI sample
// 11 : sample_11_usb_func_audio_1     USBAudio sample
// 12 : sample_12_usb_func_audio_2     USBAudio and SSIF sample
// 13 : sample_13_ether_http           Ether HTTP sample
// 14 : sample_14_ether_https          Ether HTTPS sample
// 15 : sample_15_ceu_lcd_pwm          CEU, LCD and PWM sample
// 16 : sample_16_usb_func_msd_1       USBMSD and FlashAPI sample
// 17 : sample_17_usb_func_msd_2       USBMSD and FlashAPI sample advanced version
// 18 : sample_18_mipi_drp_lcd         MIPI, DRP and LCD sample
// 19 : sample_19_mipi_drp_diplayapp   MIPI, DRP and USBSerial (CDC) sample (use "DisplayApp")
// 20 : sample_20_drp_dynamic_loading  DRP Dynamic Loading Sample
// 21 : sample_21_deep_standby_alarm   Deep standby and RTC alarm sample

#endif // SAMPLE_SELECT_H
