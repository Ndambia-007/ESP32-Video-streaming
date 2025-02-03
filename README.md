# ESP32 Mjpeg Video Streaming 
## Video conversion

-There is an ESP32 with a built in 320 x 240 2.8" LCD display with a touch screen called the "ESP32-2432S028R", since this doesn't roll of the tongue, I propose it should be renamed the "Cheap Yellow Display" or CYD for short.
The project use FFmpeg convert the video into ESP32 readable format.

### Please download and install FFmpeg at their official site if not yet: 
```C
https://ffmpeg.org

```
### Convert to Motion JPEG
```C

ffmpeg -i input.mp4 -vf "fps=30,scale=-1:176:flags=lanczos,crop=220:in_h:(in_w-220)/2:0" -q:v 9 220_30fps.mjpeg
```


Bus initialization VSPI for SD card and HSPI for TFT screen and touch control

```C
SPIClass vspi(VSPI);
SPIClass hspi(HSPI);

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 1);
```


### TFT PIN Description
* #define TFT_MISO 12
* #define TFT_MOSI 13
* #define TFT_SCLK 14
* #define TFT_CS   15
* #define TFT_DC    2
* #define TFT_RST   4
* #define TFT_BL   21

### SD PIN Description
* #define SD_MISO 19
* #define SD_MOSI 23
* #define SD_SCLK 18
* #define SD_CS    5  // Chip select for SD card

### Touch Pin Descrption
* #define XPT2046_IRQ 36
* #define XPT2046_MOSI 32
* #define XPT2046_MISO 39
* #define XPT2046_CLK 25
* #define XPT2046_CS 33



![20250203_163615](https://github.com/user-attachments/assets/e403ffdb-b00e-45e2-938e-0dd524524f93)
![20250203_164435](https://github.com/user-attachments/assets/a4a5fb49-7d53-4b4e-b0f6-fa370a0afa76)
![20250203_164428](https://github.com/user-attachments/assets/29a0305b-5103-4147-a67a-b6d8afcdbb64)
