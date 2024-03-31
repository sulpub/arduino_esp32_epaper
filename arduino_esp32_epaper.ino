/***
   MQTT connected e-paper

   Board : Lilygo T5_V2.3-2.13
   source : https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series
   exemple : https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series/blob/master
   prodcut : https://www.aliexpress.com/item/32869729970.html
   schematic : https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series/blob/master/schematic/T5V2.3.pdf
   
   Source :
    - 2021 by martin schlatter, schwetzingen, germany
*/

#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>

// According to the board, cancel the corresponding macro definition
#define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28
// #define LILYGO_T5_V102
// #define LILYGO_T5_V266
// #define LILYGO_EPD_DISPLAY         //T-Display 1.02 inch epaper   //Depend  https://github.com/adafruit/Adafruit_NeoPixel
// #define LILYGO_EPD_DISPLAY_154

//#include <boards.h>
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>

#if defined(LILYGO_T5_V102) || defined(LILYGO_EPD_DISPLAY)
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include <Adafruit_NeoPixel.h>             //Depend  https://github.com/adafruit/Adafruit_NeoPixel
#elif defined(LILYGO_T5_V266)
#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
#elif defined(LILYGO_T5_V213)
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#else
#endif

#include GxEPD_BitmapExamples

//#include "DejaVuSans9pt7b.h"
#include "FreeMonoBold9pt7b.h"
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#define  EPD_CS 5
#define  EPD_DC 17
#define  EPD_RSET 16
#define EPD_BUSY 4
#define EPD_SCLK 18
#define EPD_MISO 2l
#define EPD_MOSI 23

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
SPIClass SDSPI(VSPI);
#endif

#if defined(_GxGDEW0213Z16_H_) || defined(_GxGDEW029Z10_H_) || defined(_GxGDEW027C44_H_) ||defined(_GxGDEW0154Z17_H_) || defined(_GxGDEW0154Z04_H_) || defined(_GxDEPG0290R_H_)
#define _HAS_COLOR_
#endif

#if defined(LILYGO_EPD_DISPLAY)
Adafruit_NeoPixel strip(RGB_STRIP_COUNT, RGB_STRIP_PIN, NEO_GRBW + NEO_KHZ800);
#endif /*LILYGO_EPD_DISPLAY_102*/

void setup() {
#if defined(LILYGO_EPD_DISPLAY)
  pinMode(EPD_POWER_ENABLE, OUTPUT);
  digitalWrite(EPD_POWER_ENABLE, HIGH);
  delay(50);
  // strip test
  strip.begin();
  strip.show();
  strip.setBrightness(200);
  int i = 0;
  while (i < 5) {
    uint32_t color[] = {0xFF0000, 0x00FF00, 0x0000FF, 0x000000};
    strip.setPixelColor(0, color[i]);
    strip.show();
    delay(1000);
    i++;
  }
  strip.setPixelColor(0, 0);
  strip.show();
#endif /*LILYGO_EPD_DISPLAY*/

#if defined(LILYGO_EPD_DISPLAY_102)
  pinMode(EPD_POWER_ENABLE, OUTPUT);
  digitalWrite(EPD_POWER_ENABLE, HIGH);
#endif /*LILYGO_EPD_DISPLAY_102*/
#if defined(LILYGO_T5_V102)
  pinMode(POWER_ENABLE, OUTPUT);
  digitalWrite(POWER_ENABLE, HIGH);
#endif /*LILYGO_T5_V102*/

  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

  //    display.init();
  //    display.setTextColor(GxEPD_BLACK);
  //    display.setRotation(1);
}


void loop() {

  display.init();
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  //display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  //display.setFont(&FreeSans9pt7b);
  //display.setFont(&DejaVuSans9pt7b);
  long z = random(0, 20);
  display.setCursor(0, z); // leicht nach unten verschieben gegen einbrennen
  display.println();
  display.println("s1");
  display.println("s2");
  display.println("s3");
  display.println("s4");
  display.update();
  display.powerDown();

  while (1);

}
