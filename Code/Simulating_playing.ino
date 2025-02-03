#include <Arduino_GFX_Library.h>
#include <XPT2046_Touchscreen.h>
#include <SD.h>
#include <SPI.h>


#include "Aurek_Besh_Narrow6pt7b.h"

#include <WiFi.h>
#define MJPEG_FILENAME "/one.mjpeg"
// Define TFT pins
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define TFT_BL   21            // LED back-light control pin

#define SD_MISO 19
#define SD_MOSI 23
#define SD_SCLK 18
#define SD_CS    5  // Chip select for SD card

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define TFT_RED    0xF800
#define TFT_GREEN  0x07E0
#define TFT_BLUE   0x001F
#define TFT_WHITE  0xFFFF
#define TFT_BLACK  0x0000

#define touchscreen_Min_X 366
#define touchscreen_Max_X 3714
#define touchscreen_Min_Y 208
#define touchscreen_Max_Y 3772

#define MJPEG_BUFFER_SIZE (220 * 176 * 2 / 4)
///#define MJPEG_BUFFER_SIZE 1024

#define MJPEG_FILENAME "/one.mjpeg"
#define MJPEG_FILENAME2 "/two.mjpeg"

SPIClass vspi(VSPI);
SPIClass hspi(HSPI);

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 1);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

int button1X = 50, button1Y = 35, buttonWidth = 190, buttonHeight = 50;
int button2X = 50, button2Y = 130;

enum SystemState { IDLE, MENU, VIDEO_PLAYBACK };
SystemState currentState = IDLE;

#include "MjpegClass.h"
static MjpegClass mjpeg;
void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  
  pinMode(SD_CS, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(XPT2046_CS, OUTPUT);
  pinMode(XPT2046_IRQ, INPUT_PULLUP);

  pinMode(TFT_BL, OUTPUT);
  //backlight on
  digitalWrite(TFT_BL, HIGH);

  digitalWrite(XPT2046_CS, HIGH); // Ensure touchscreen is disabled initially
  digitalWrite(TFT_CS, HIGH);     // Ensure TFT is not selected when initializing other peripherals

  gfx->begin();
  gfx->fillScreen(TFT_BLACK);
  gfx->setTextSize(1);  
  gfx->setTextColor(TFT_WHITE);
  gfx->setFont(&Aurek_Besh_Narrow6pt7b); // Adjust size accordingly

  hspi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  vspi.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, vspi)) {
    Serial.println(F("ERROR: SD card mount failed!"));
    gfx->println(F("ERROR: SD card mount failed!"));
    return;
  }

  Serial.println("SD Card initialized.");
  ts.begin(hspi);
  ts.setRotation(3);

  drawMenuButtons();
}
void loop() {
    if (currentState == IDLE && ts.tirqTouched() && ts.touched()) {
        TS_Point p = ts.getPoint();
        int x = constrain(map(p.x, touchscreen_Min_X, touchscreen_Max_X, 0, SCREEN_WIDTH), 0, SCREEN_WIDTH);
        int y = constrain(map(p.y, touchscreen_Min_Y, touchscreen_Max_Y, 0, SCREEN_HEIGHT), 0, SCREEN_HEIGHT);

        Serial.print(x); Serial.print(", "); Serial.println(y);

        if ((x > 20 && x < 145) && (y > 72 && y < 177)) {
            Serial.println("Button 1 pressed: Play Video 1");
            feedbackOnButtonPress(button1X, button1Y, "Play Video 1", TFT_RED);
            currentState = VIDEO_PLAYBACK;
            playVideo(MJPEG_FILENAME);
        } else if ((x > 20 && x < 177) && (y > 204 && y < 283)) {
            Serial.println("Button 2 pressed: Play Video 2");
            feedbackOnButtonPress2(button2X, button2Y, "Play Video 2", TFT_RED);
            currentState = VIDEO_PLAYBACK;
            playVideo(MJPEG_FILENAME2);
        }
    }
}



void playVideo(const char *filename) {
    Serial.println(F("Starting video playback"));
    File vFile = SD.open(filename);
    if (!vFile || vFile.isDirectory()) {
        Serial.println(F("ERROR: Failed to open video file for reading"));
        return;
    }

    uint8_t *mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
    if (!mjpeg_buf) {
        Serial.println(F("ERROR: mjpeg_buf malloc failed!"));
        vFile.close();
        return;
    }

    Serial.println(F("MJPEG video start"));
    mjpeg.setup(vFile, mjpeg_buf, (Arduino_TFT *)gfx, true);

    while (mjpeg.readMjpegBuf()) {
        // Draw video frame
        mjpeg.drawJpg();
        yield();  // Prevent watchdog reset

        // Check for touch during playback
        if (ts.tirqTouched() && ts.touched()) {
            Serial.println("Touch detected! Stopping video...");
            break;  // Exit the loop and stop video playback
        }
    }

    Serial.println(F("MJPEG video end"));
    vFile.close();
    free(mjpeg_buf);
    mjpeg_buf = NULL;
    delay(500);

    // Return to main menu
    gfx->fillScreen(TFT_BLACK);
    drawMenuButtons();
    currentState = IDLE;
}


void drawMenuButtons() {
  gfx->fillRect(button1X, button1Y, buttonWidth, buttonHeight, TFT_BLUE);
  gfx->setTextColor(TFT_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(70, 65);
  gfx->print("Play Video 1");

  gfx->fillRect(button2X, button2Y, buttonWidth, buttonHeight, TFT_GREEN);
  gfx->setCursor(70, 165);
  gfx->print("Play Video 2");
}
void feedbackOnButtonPress(int x, int y, const char *label, uint16_t color) {
  gfx->fillRect(button1X, button1Y, buttonWidth, buttonHeight, TFT_BLACK);
  gfx->fillRect(x-20 , y-15, buttonWidth, buttonHeight, color);
  gfx->setTextColor(TFT_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 20, y + 15);
  gfx->print(label);
  delay(500);
  gfx->fillRect(x-20, y-15, buttonWidth+40, buttonHeight+20, TFT_BLACK); 
  drawMenuButtons(); // Redraw buttons after feedback
}

void feedbackOnButtonPress2(int x, int y, const char *label, uint16_t color) {
  gfx->fillRect(button2X, button2Y, buttonWidth, buttonHeight, TFT_BLACK);
  gfx->fillRect(x-20 , y-15, buttonWidth, buttonHeight, color);
  gfx->setTextColor(TFT_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(x + 20, y + 15);
  gfx->print(label);
  delay(500);
  gfx->fillRect(x-20, y-15, buttonWidth+40, buttonHeight+20, TFT_BLACK); 
  drawMenuButtons(); // Redraw buttons after feedback
}
