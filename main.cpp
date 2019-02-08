
// About terminal output such as printf()
// The UART baud rate when printf() is output to the terminal is 115,200.
// You can change the baud rate by "platform.stio-baud-rate" of "mbed_app.json".

// To use TARGET_RZ_A2M_EVB, set the dip switch SW6 of the SUB board as follows.
//   SW6-1  OFF
//   SW6-2  OFF
//   SW6-3  OFF
//   SW6-4  OFF
//   SW6-5  ON
//   SW6-6  ON
//   SW6-7  ON
//   SW6-8  OFF
//   SW6-9  OFF
//   SW6-10 OFF

// You can try each sample program by changing the following macro.
#define SAMPLE_PROGRAM_NO  0  //  0 : DigitalOut, InterruptIn, RTC, Timer and AnalogIn sample
                              //  1 : FlashAPI sample
                              //  2 : SSIF loop back sample
                              //  3 : SPDIF loop back sample
                              //  4 : SSIF wav playback sample (use USB memory or SD card)
                              //  5 : SPDIF wav playback sample (use USB memory or SD card)
                              //  6 : LCD, Touch panel and JCU sample (use USB memory or SD card)
                              //  7 : USBSerial (CDC) sample
                              //  8 : USBMouse sample
                              //  9 : USBKeyboard sample
                              // 10 : USBMIDI sample
                              // 11 : USBAudio sample
                              // 12 : USBAudio and SSIF sample
                              // 13 : Ether HTTP sample
                              // 14 : Ether HTTPS sample
                              // 15 : CEU, LCD and PWM sample
                              // 16 : USBMSD and FlashAPI sample
                              // 17 : USBMSD and FlashAPI sample advanced version
                              // 18 : MIPI and LCD sample
                              // 19 : DRP Basic Operation Sample
                              // 20 : DRP Dynamic Loading Sample


#if (SAMPLE_PROGRAM_NO == 0)  //-----------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  0 : DigitalOut, InterruptIn, RTC, Timer and AnalogIn sample
//
// LED1 will blink at 1 second intervals.
// While pressing USER_BUTTON 0, LED2 lights up.
// RTC value, Timer value and AD value are displayed every 1 second.

#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
InterruptIn button(USER_BUTTON0);
#if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_SBEV)
AnalogIn adin(P5_6); // In TARGET_RZ_A2M_EVB, SW5 and SW4 are connected to P5_6 as AD switches.
#else
AnalogIn adin(A0);   // For other Mbed boards (GR-PEACH, GR-LYCHEE)
#endif

static void button_fall(void) {
    led2 = !led2;
}

static void button_rise(void) {
    led2 = 0;
}

int main(void) {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

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


#elif (SAMPLE_PROGRAM_NO == 1)  //---------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  1 : FlashAPI sample
//
// This is a sample program of flash writing.
// Writes to the address set in FLASH_ADDR.
// Erasing is in units of 4096 bytes.

#include "mbed.h"

#define FLASH_ADDR  0x20700000

static uint8_t flash_work_buff[256];

static void disp_flash_work_buff(void) {
    for (uint32_t i = 0; i < sizeof(flash_work_buff); i++) {
        if ((i != 0) && ((i & 0x0F) == 0x00)) {
            printf("\r\n");
        }
        printf("%02x ", flash_work_buff[i]);
    }
    printf("\r\n\r\n");
}

int main(void) {
    int ret;
    uint32_t i;
    FlashIAP flash;

    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    flash.init();

    // read
    memset(flash_work_buff, 0, sizeof(flash_work_buff));
    ret = flash.read(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.read ret=%d\r\n", (int)ret);
    disp_flash_work_buff();

    // erase
    ret = flash.erase(FLASH_ADDR, flash.get_sector_size(FLASH_ADDR));
    printf("flash.erase ret=%d\r\n", (int)ret);

    // read
    memset(flash_work_buff, 0, sizeof(flash_work_buff));
    ret = flash.read(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.read ret=%d\r\n", (int)ret);
    disp_flash_work_buff();

    // write
    for (i = 0; i < sizeof(flash_work_buff); i++) {
        flash_work_buff[i] = i;
    }
    ret = flash.program(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.write ret=%d\r\n", (int)ret);

    // read
    memset(flash_work_buff, 0, sizeof(flash_work_buff));
    ret = flash.read(flash_work_buff, FLASH_ADDR, sizeof(flash_work_buff));
    printf("flash.read ret=%d\r\n", (int)ret);
    disp_flash_work_buff();

    printf("SAMPLE_PROGRAM_NO %d end\r\n", SAMPLE_PROGRAM_NO);
    while (1);
}


#elif (SAMPLE_PROGRAM_NO == 2) || (SAMPLE_PROGRAM_NO == 3)  //------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  2 : SSIF loop back sample
// SAMPLE_PROGRAM_NO  3 : SPDIF loop back sample
//
// The input sound is output as it is.
// In case of SAMPLE_PROGRAM_NO=2, please input the sound from the line-in, not from the microphone.
// In case of SAMPLE_PROGRAM_NO=3 :
//   Since TARGET_RZ_A2M_EVB has no connector and no suitable AUDIO_CLK has been entered, only pseudo-evaluation is executed.
//   - Please connect PC_5 and PC_4 when evaluating.
//   - Since AUDIO_CLK on the board is 11.2896 MHz, it operates at half the actual speed.
//   - The AUDIO_CLK frequency required is 512 times as large as the sample frequency for audio data.
//     (fs=44.1kHz : AUDIO_CLK=22.5792MHz, fs=48kHz : AUDIO_CLK=24.5760MHz)

#if ((SAMPLE_PROGRAM_NO == 3) && !defined(TARGET_RZ_A2M_EVB))
  #error "SPDIF is not supported."
#endif

#include "mbed.h"

#define WRITE_BUFF_NUM         (8)
#define READ_BUFF_NUM          (8)
#define MAIL_QUEUE_SIZE        (WRITE_BUFF_NUM + READ_BUFF_NUM + 1)
#define INFO_TYPE_WRITE_END    (0)
#define INFO_TYPE_READ_END     (1)

#if (SAMPLE_PROGRAM_NO == 2)
  #include "AUDIO_GRBoard.h"
  #define AUDIO_BUFF_SIZE      (4096)
  AUDIO_GRBoard audio(0x80, WRITE_BUFF_NUM, READ_BUFF_NUM);
#else
  #include "SPDIF_RBSP.h"
  #define AUDIO_BUFF_SIZE      (192 * 2 * 10) // 1 block * 2ch * 10
  SPDIF_RBSP audio(P6_4, PC_5, PC_4, false, 0x80, WRITE_BUFF_NUM, READ_BUFF_NUM);
#endif

typedef struct {
    uint32_t info_type;
    void *   p_data;
    int32_t  result;
} mail_t;
Mail<mail_t, MAIL_QUEUE_SIZE> mail_box;

//32 bytes aligned! No cache memory
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t audio_read_buff[READ_BUFF_NUM][AUDIO_BUFF_SIZE] @ ".mirrorram";
#else
static uint8_t audio_read_buff[READ_BUFF_NUM][AUDIO_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));
#endif

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

    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    audio.power();                // For GR-PEACH and GR-LYCHEE
    audio.outputVolume(0.5, 0.5); // For GR-PEACH and GR-LYCHEE
    audio.micVolume(0.7);         // For GR-PEACH and GR-LYCHEE

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


#elif (SAMPLE_PROGRAM_NO == 4) || (SAMPLE_PROGRAM_NO == 5)  //------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  4 : SSIF wav playback sample (use USB memory or SD card)
// SAMPLE_PROGRAM_NO  5 : SPDIF wav playback sample (use USB memory or SD card)
//
// This sample will play a ".wav" file of the SD root folder.
// Wav file format : RIFF format, 1ch or 2ch, 44.1kHz, 16bits
// In case of SAMPLE_PROGRAM_NO=5 :
//   Since TARGET_RZ_A2M_EVB has no connector and no suitable AUDIO_CLK has been entered, only pseudo-evaluation is executed.
//   - Audio data is output from the PC_5 terminal.
//   - Since AUDIO_CLK on the board is 11.2896 MHz, it operates at half the actual speed.
//   - The AUDIO_CLK frequency required is 512 times as large as the sample frequency for audio data.
//     (fs=44.1kHz : AUDIO_CLK=22.5792MHz, fs=48kHz : AUDIO_CLK=24.5760MHz)

#if ((SAMPLE_PROGRAM_NO == 5) && !defined(TARGET_RZ_A2M_EVB))
  #error "SPDIF is not supported."
#endif

#include "mbed.h"
#include "FATFileSystem.h"
#include "SdUsbConnect.h"
#include "EasyPlayback.h"
#include "EasyDec_WavCnv2ch.h"

#if (SAMPLE_PROGRAM_NO == 4)
  static EasyPlayback AudioPlayer;
#else
  static EasyPlayback AudioPlayer(EasyPlayback::AUDIO_TPYE_SPDIF);
#endif

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

    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    // decoder setting
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".wav");
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".WAV");

    AudioPlayer.outputVolume(0.5);  // For GR-PEACH and GR-LYCHEE

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


#elif (SAMPLE_PROGRAM_NO == 6)  //---------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  6 : LCD, Touch panel and JCU sample (use USB memory or SD card)
//
// Decodes the JPEG file in the USB memory and displays it on the LED.
// Touch information displays on the JPEG image.
// Up to 2 touch positions can be recognized at the same time.

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

    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

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


#elif (SAMPLE_PROGRAM_NO == 7)  //---------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  7 : USBSerial (CDC) sample
// 
// USBSerial example
// Virtual serial port over USB
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBSerial.h"

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    USBSerial serial;

    wait(0.1);
    serial.printf("I am a virtual serial port\r\n");

    // echoback
    while (1) {
        if (serial.readable()) {
            serial._putc(serial._getc());
        }
    }
}


#elif (SAMPLE_PROGRAM_NO == 8)  //---------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  8 : USBMouse sample
//
// The USBMouse interface is used to emulate a mouse over the USB port. 
// You can choose relative or absolute co-ordinates, and send clicks, button state and scroll wheel movements.
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBMouse.h"

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    USBMouse mouse;
    int16_t x = 0;
    int16_t y = 0;
    int32_t radius = 10;
    int32_t angle = 0;

    while (1) {
        x = cos((double)angle*3.14/180.0)*radius;
        y = sin((double)angle*3.14/180.0)*radius;

        mouse.move(x, y);
        angle += 3;
        wait(0.001);
    }
}


#elif (SAMPLE_PROGRAM_NO == 9)  //---------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO  9 : USBKeyboard sample
//
// The USBKeyboard interface is used to emulate a keyboard over the USB port.
// You can type strings and send keycodes.
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBKeyboard.h"

int main(void) {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    USBKeyboard key;

    while (1) {
        key.printf("Hello World\r\n");
        wait(1);
    }
}


#elif (SAMPLE_PROGRAM_NO == 10)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 10 : USBMIDI sample
//
// The USBMIDI interface can be used to send and receive MIDI messages over USB using the standard USB-MIDI protocol.
// Using this library, you can do things like send MIDI messages to a computer (such as to record in a sequencer,
// or trigger a software synthesiser) and receive messages from a computer (such as actuate things based on MIDI events)
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBMIDI.h"

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    USBMIDI midi;

    while (1) {
        for(int i = 48; i < 83; i++) {     // send some messages!
            midi.write(MIDIMessage::NoteOn(i));
            wait(0.25);
            midi.write(MIDIMessage::NoteOff(i));
            wait(0.5);
        }
    }
}


#elif (SAMPLE_PROGRAM_NO == 11)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 11 : USBAudio sample
//
// Audio loopback example use:
// 1. Select "Mbed Audio" as your sound device
// 2. Play a song or audio file
// 3. Record the output using a program such as Audacity
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBAudio.h"

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    USBAudio audio(true, 44100, 2, 44100, 2);

    printf("Looping audio\r\n");
    static uint8_t buf[128];
    while (true) {
        if (!audio.read(buf, sizeof(buf))) {
            memset(buf, 0, sizeof(buf));
        }
        audio.write(buf, sizeof(buf));
    }
}


#elif (SAMPLE_PROGRAM_NO == 12)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 12 : USBAudio and SSIF sample
// Sound from PC will be output to SSIF. (speaker)
// Sound from SSIF will be input to the PC. (microphone)
// [Attention!] Delete the "OVERRIDE_CONSOLE_USBSERIAL" macro in "mbed_app.json" file if it is set.

#include "mbed.h"
#include "USBAudio.h"
#include "AUDIO_GRBoard.h"

#define AUDIO_OUT_BUFF_NUM     (8)
#define AUDIO_IN_BUFF_NUM      (8)
#define AUDIO_BUFF_SIZE        (4096)
AUDIO_GRBoard i2s_audio(0x80, AUDIO_OUT_BUFF_NUM - 1, AUDIO_IN_BUFF_NUM);

//32 bytes aligned! No cache memory
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t audio_out_buff[AUDIO_OUT_BUFF_NUM][AUDIO_BUFF_SIZE] @ ".mirrorram";
#pragma data_alignment=32
static uint8_t audio_in_buff[AUDIO_IN_BUFF_NUM][AUDIO_BUFF_SIZE] @ ".mirrorram";
#else
static uint8_t audio_out_buff[AUDIO_OUT_BUFF_NUM][AUDIO_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));
static uint8_t audio_in_buff[AUDIO_IN_BUFF_NUM][AUDIO_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));
#endif

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
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

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


#elif (SAMPLE_PROGRAM_NO == 13)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 13 : Ether HTTP sample

#include "mbed.h"
#include "http_request.h"
#include "EthernetInterface.h"

EthernetInterface eth;

void dump_response(HttpResponse* res) {
    printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    NetworkInterface* network = &eth;

    printf("\nConnecting...\n");
    int ret = network->connect();
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
    printf("Success\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    printf("IP: %s\n", network->get_ip_address());
    printf("Netmask: %s\n", network->get_netmask());
    printf("Gateway: %s\n", network->get_gateway());

    // Do a GET request to httpbin.org
    {
        // By default the body is automatically parsed and stored in a buffer, this is memory heavy.
        // To receive chunked response, pass in a callback as last parameter to the constructor.
        HttpRequest* get_req = new HttpRequest(network, HTTP_GET, "http://httpbin.org/status/418");

        HttpResponse* get_res = get_req->send();
        if (!get_res) {
            printf("HttpRequest failed (error code %d)\n", get_req->get_error());
            return 1;
        }

        printf("\n----- HTTP GET response -----\n");
        dump_response(get_res);

        delete get_req;
    }

    // POST request to httpbin.org
    {
        HttpRequest* post_req = new HttpRequest(network, HTTP_POST, "http://httpbin.org/post");
        post_req->set_header("Content-Type", "application/json");

        const char body[] = "{\"hello\":\"world\"}";

        HttpResponse* post_res = post_req->send(body, strlen(body));
        if (!post_res) {
            printf("HttpRequest failed (error code %d)\n", post_req->get_error());
            return 1;
        }

        printf("\n----- HTTP POST response -----\n");
        dump_response(post_res);

        delete post_req;
    }

    wait(osWaitForever);
}


#elif (SAMPLE_PROGRAM_NO == 14)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 14 : Ether HTTPS sample

#include "mbed.h"
#include "https_request.h"
#include "EthernetInterface.h"

EthernetInterface eth;

/* List of trusted root CA certificates
 *
 * - Amazon, the CA for os.mbed.com
 * - Let's Encrypt, the CA for httpbin.org
 * - Comodo, the CA for reqres.in
 *
 * To add more root certificates, just concatenate them.
 */
const char SSL_CA_PEM[] =  "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIICiTCCAg+gAwIBAgIQH0evqmIAcFBUTAGem2OZKjAKBggqhkjOPQQDAzCBhTEL\n"
    "MAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n"
    "BxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMT\n"
    "IkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDgwMzA2MDAw\n"
    "MDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdy\n"
    "ZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09N\n"
    "T0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlv\n"
    "biBBdXRob3JpdHkwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQDR3svdcmCFYX7deSR\n"
    "FtSrYpn1PlILBs5BAH+X4QokPB0BBO490o0JlwzgdeT6+3eKKvUDYEs2ixYjFq0J\n"
    "cfRK9ChQtP6IHG4/bC8vCVlbpVsLM5niwz2J+Wos77LTBumjQjBAMB0GA1UdDgQW\n"
    "BBR1cacZSBm8nZ3qQUfflMRId5nTeTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/\n"
    "BAUwAwEB/zAKBggqhkjOPQQDAwNoADBlAjEA7wNbeqy3eApyt4jf/7VGFAkK+qDm\n"
    "fQjGGoe9GKhzvSbKYAydzpmfz1wPMOG+FDHqAjAU9JM8SaczepBGR7NjfRObTrdv\n"
    "GDeAU/7dIOA1mjbRxwG55tzd8/8dLDoWV9mSOdY=\n"
    "-----END CERTIFICATE-----\n";

void dump_response(HttpResponse* res) {
    mbedtls_printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    mbedtls_printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        mbedtls_printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    mbedtls_printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    NetworkInterface* network = &eth;

    printf("\nConnecting...\n");
    int ret = network->connect();
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
    printf("Success\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    printf("IP: %s\n", network->get_ip_address());
    printf("Netmask: %s\n", network->get_netmask());
    printf("Gateway: %s\n", network->get_gateway());

    // GET request to developer.mbed.org
    {
        printf("\n----- HTTPS GET request -----\n");

        HttpsRequest* get_req = new HttpsRequest(network, SSL_CA_PEM, HTTP_GET, "https://os.mbed.com/media/uploads/mbed_official/hello.txt");

        HttpResponse* get_res = get_req->send();
        if (!get_res) {
            printf("HttpRequest failed (error code %d)\n", get_req->get_error());
            return 1;
        }
        printf("\n----- HTTPS GET response -----\n");
        dump_response(get_res);

        delete get_req;
    }

    // POST request to httpbin.org
    {
        printf("\n----- HTTPS POST request -----\n");

        HttpsRequest* post_req = new HttpsRequest(network, SSL_CA_PEM, HTTP_POST, "https://httpbin.org/post");
        post_req->set_header("Content-Type", "application/json");

        const char body[] = "{\"hello\":\"world\"}";

        HttpResponse* post_res = post_req->send(body, strlen(body));
        if (!post_res) {
            printf("HttpRequest failed (error code %d)\n", post_req->get_error());
            return 1;
        }

        printf("\n----- HTTPS POST response -----\n");
        dump_response(post_res);

        delete post_req;
    }

    wait(osWaitForever);
}


#elif (SAMPLE_PROGRAM_NO == 15)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 15 : CEU, LCD and PWM sample
//
// Display camera image on LCD
// Pressing SW3 changes the LCD backlight. (PWM)
//
// Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_MT9V111", "CAMERA_OV7725", "CAMERA_OV5642" or "CAMERA_WIRELESS_CAMERA".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_XXXXXXXX"
//        },
//
// Please set the value of "lcd-type" in "mbed_app.json" to "GR_PEACH_xxx", "GR_LYCHEE_xxx" or "RZ_A2M_EVB_RSK_TFT".
//
//        "lcd-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "RZ_A2M_EVB_RSK_TFT"
//        },


#include "mbed.h"
#include "EasyAttach_CameraAndLCD.h"

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640)
#define VIDEO_PIXEL_VW         (480)

#define DATA_SIZE_PER_PIC      (2u)
#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

DisplayBase Display;

#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT] @ ".mirrorram";
#else
static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
#endif

static void Start_Video_Camera(void) {
    // Video capture setting (progressive form fixed)
    Display.Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)user_frame_buffer0,
        FRAME_BUFFER_STRIDE,
        DisplayBase::VIDEO_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        VIDEO_PIXEL_VW,
        VIDEO_PIXEL_HW
    );
    EasyAttach_CameraStart(Display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;

#if (LCD_PIXEL_HEIGHT > VIDEO_PIXEL_VW)
    rect.vs = (LCD_PIXEL_HEIGHT - VIDEO_PIXEL_VW) / 2;  // centering
#else
    rect.vs = 0;
#endif
    rect.vw = VIDEO_PIXEL_VW;
#if (LCD_PIXEL_WIDTH > VIDEO_PIXEL_HW)
    rect.hs = (LCD_PIXEL_WIDTH - VIDEO_PIXEL_HW) / 2;   // centering
#else
    rect.hs = 0;
#endif
    rect.hw = VIDEO_PIXEL_HW;
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

static void button_fall(void) {
    static uint32_t backlight_val = 10;

    if (backlight_val > 0) {
        backlight_val--;
    } else {
        backlight_val = 10;
    }
    EasyAttach_LcdBacklight((float)(0.1f * backlight_val));
}

int main(void) {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    EasyAttach_Init(Display);
    Start_LCD_Display();
    Start_Video_Camera();

    InterruptIn button(USER_BUTTON0);
    button.fall(&button_fall);

    wait(osWaitForever);
}


#elif (SAMPLE_PROGRAM_NO == 16)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 16 : USBMSD and FlashAPI sample
//
// USBMSD and FlashAPI sample.
// It is a sample program that can read and write serial flash from the PC via Filesystem.
// By connecting the board and the PC with a USB cable, you can refer inside the device as a mass storage device.
// (The format is required when connecting for the first time.)
// Part of the flash address is used as storage.
// The first 1MB is used as the program area, and the rest is used as the storage.

#include "mbed.h"
#include "USBMSD.h"
#include "FlashIAPBlockDevice.h"
#include "mbed_drv_cfg.h"

int main() {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    FlashIAPBlockDevice flashiap_bd(FLASH_BASE + 0x100000, FLASH_SIZE - 0x100000);
    USBMSD usb(&flashiap_bd);

    while (true) {
        usb.process();
    }
}


#elif (SAMPLE_PROGRAM_NO == 17)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 17 : USBMSD and FlashAPI sample advanced version
//
// USBMSD and FlashAPI sample advanced version
// It is a sample program that can read and write serial flash from the PC via Filesystem.
// By connecting the board and the PC with a USB cable, you can refer inside the device as a mass storage device.
// (The format is required when connecting for the first time.)
// When you write a JPEG file (.jpg) from the PC to the storage, the image is displayed.
// By pressing SW3, change the save destination to heap memory.

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

    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

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


#elif (SAMPLE_PROGRAM_NO == 18)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 18 : MIPI and LCD sample
//
// Display MIPI camera image on LCD
//
// Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_RASPBERRY_PI".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_RASPBERRY_PI"
//        },

#if !defined(TARGET_RZ_A2XX)
#error "MIPI is not supported."
#endif
#if MBED_CONF_APP_CAMERA_TYPE != CAMERA_RASPBERRY_PI
#error Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_RASPBERRY_PI" and build.
#endif

#include "mbed.h"
#include "EasyAttach_CameraAndLCD.h"

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640)
#define VIDEO_PIXEL_VW         (480)

#define DATA_SIZE_PER_PIC      (1u)
#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

DisplayBase Display;

static uint8_t user_frame_buffer0[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(128)));

static void Start_Video_Camera(void) {
    // Video capture setting (progressive form fixed)
    Display.Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)user_frame_buffer0,
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

    rect.vs = 0;
    rect.vw = VIDEO_PIXEL_VW;
    rect.hs = 0;
    rect.hw = VIDEO_PIXEL_HW;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)user_frame_buffer0,
        FRAME_BUFFER_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_CLUT8,
        DisplayBase::WR_RD_WRSWA_32_16_8BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

    ThisThread::sleep_for(50);
    EasyAttach_LcdBacklight(true);
}

int main(void) {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    EasyAttach_Init(Display);
    Start_LCD_Display();
    Start_Video_Camera();

    wait(osWaitForever);
}


#elif (SAMPLE_PROGRAM_NO == 19)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 19 : DRP Basic Operation Sample,
//
// Converts the input image from MIPI camera to YUV image using DRP and outputs to display.
// Please refer to the document for details. (RZ_A2M_Mbed\drp-for-mbed\r_drp\doc),
//
// Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_RASPBERRY_PI".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_RASPBERRY_PI"
//        },

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

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640)
#define VIDEO_PIXEL_VW         (480)

#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * 1) + 31u) & ~31u)
#define FRAME_BUFFER_STRIDE_2  (((VIDEO_PIXEL_HW * 2) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

#define DRP_FLG_CAMER_IN       (0x00000001)

static DisplayBase Display;
static uint8_t fbuf_bayer[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(128)));
static uint8_t fbuf_yuv[FRAME_BUFFER_STRIDE_2 * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
#if defined(__ICCARM__)
static r_drp_simple_isp_t param_isp @ ".mirrorram";
#else
static r_drp_simple_isp_t param_isp __attribute((section("NC_BSS")));
#endif
static uint8_t drp_lib_id[R_DK2_TILE_NUM] = {0};
static Thread drpTask(osPriorityHigh);

static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type) {
    drpTask.flags_set(DRP_FLG_CAMER_IN);
}

static void cb_drp_finish(uint8_t id) {
    // do nothing
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

    rect.vs = 0;
    rect.vw = VIDEO_PIXEL_VW;
    rect.hs = 0;
    rect.hw = VIDEO_PIXEL_HW;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)fbuf_yuv,
        FRAME_BUFFER_STRIDE_2,
        DisplayBase::GRAPHICS_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16_8BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);

    ThisThread::sleep_for(50);
    EasyAttach_LcdBacklight(true);
}

static void drp_task(void) {
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
    param_isp.gain_r = 0x1500;
    param_isp.gain_g = 0x1000;
    param_isp.gain_b = 0x18E3;

    while (true) {
        ThisThread::flags_wait_all(DRP_FLG_CAMER_IN);
        R_DK2_Start(drp_lib_id[0], (void *)&param_isp, sizeof(r_drp_simple_isp_t));
    }
}

int main(void) {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    // Start DRP task
    drpTask.start(callback(drp_task));

    EasyAttach_Init(Display);
    Start_LCD_Display();
    // Interrupt callback function setting (Field end signal for recording function in scaler 0)
    Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);
    Start_Video_Camera();

    wait(osWaitForever);
}


#elif (SAMPLE_PROGRAM_NO == 20)  //--------------------------------------------------------------------------------------------
// SAMPLE_PROGRAM_NO 20 : DRP Dynamic Loading Sample
//
// Detects the edges of the input image from MIPI camera by Canny method using DRP and outputs to display.
// Please refer to the document for details. (RZ_A2M_Mbed\drp-for-mbed\r_drp\doc)
//
// Please set the value of "camera-type" in "mbed_app.json" to "CAMERA_RASPBERRY_PI".
//
//        "camera-type":{
//            "help": "Please see EasyAttach_CameraAndLCD/README.md",
//            "value": "CAMERA_RASPBERRY_PI"
//        },

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
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t fbuf_overlay[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT] @ ".mirrorram";
static uint8_t drp_work_buf[(FRAME_BUFFER_STRIDE * (FRAME_BUFFER_HEIGHT + 2)) * 2] @ ".mirrorram";
static uint8_t nc_memory[512] @ ".mirrorram";
#else
static uint8_t fbuf_overlay[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
static uint8_t drp_work_buf[(FRAME_BUFFER_STRIDE * (FRAME_BUFFER_HEIGHT + 2)) * 2]__attribute((section("NC_BSS")));
static uint8_t nc_memory[512] __attribute((section("NC_BSS")));
#endif
static uint8_t drp_lib_id[R_DK2_TILE_NUM] = {0};
static Thread drpTask(osPriorityHigh);

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
        R_DK2_Load(g_drp_lib_bayer2grayscale,
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
        R_DK2_Load(g_drp_lib_median_blur,
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
        R_DK2_Load(g_drp_lib_canny_calculate,
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
        R_DK2_Load(g_drp_lib_canny_hysterisis,
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
        sprintf(str, "Bayer2Grayscale : Load%2lu.%lums + Ran%2lu.%lums",
                drp_time[0] / 1000, (drp_time[0] - ((drp_time[0] / 1000) * 1000)) / 100,
                drp_time[1] / 1000, (drp_time[1] - ((drp_time[1] / 1000) * 1000)) / 100);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 0, 1, 2);
        sprintf(str, "MedianBlur      : Load%2lu.%lums + Ran%2lu.%lums",
                drp_time[2] / 1000, (drp_time[2] - ((drp_time[2] / 1000) * 1000)) / 100,
                drp_time[3] / 1000, (drp_time[3] - ((drp_time[3] / 1000) * 1000)) / 100);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 2, 1, 2);
        sprintf(str, "CannyCalculate  : Load%2lu.%lums + Ran%2lu.%lums",
                drp_time[4] / 1000, (drp_time[4] - ((drp_time[4] / 1000) * 1000)) / 100,
                drp_time[5] / 1000, (drp_time[5] - ((drp_time[5] / 1000) * 1000)) / 100);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 4, 1, 2);
        sprintf(str, "CannyHysterisis : Load%2lu.%lums + Ran%2lu.%lums",
                drp_time[6] / 1000, (drp_time[6] - ((drp_time[6] / 1000) * 1000)) / 100,
                drp_time[7] / 1000, (drp_time[7] - ((drp_time[7] / 1000) * 1000)) / 100);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 6, 1, 2);

        uint32_t time_sum = 0;
        for (int i = 0; i < 8; i++) {
            time_sum += drp_time[i];
        }
        sprintf(str, "Total           :%3lu.%lums",
                time_sum / 1000, (time_sum - ((time_sum / 1000) * 1000)) / 100);
        ascii_font.DrawStr(str, 5, 5 + (AsciiFont::CHAR_PIX_HEIGHT + 1) * 8, 1, 2);
    }
}

int main(void) {
    printf("SAMPLE_PROGRAM_NO %d start\r\n", SAMPLE_PROGRAM_NO);

    // Start DRP task
    drpTask.start(callback(drp_task));

    EasyAttach_Init(Display);
    Start_LCD_Display();
    // Interrupt callback function setting (Field end signal for recording function in scaler 0)
    Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);
    Start_Video_Camera();

    wait(osWaitForever);
}


#endif
