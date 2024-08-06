#include "SPI.h"
#include "Adafruit_GFX.h"         // Core graphics library
#include "Adafruit_ILI9341.h"     // Hardware-specific library
#include "SdFat.h"                // SD card & FAT filesystem library
#include "Adafruit_ImageReader.h" // Image-reading functions
#include "XPT2046_Touchscreen.h"

// For the Adafruit shield, these are the default.
#define TFT_CS 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 8
#define TFT_MISO 12

#define TS_CS 7
#define SD_CS 6

#define ROTATION 3

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC/RST
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS);

int tftWidth, tftHeight;

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

void setup() {
  
  Serial.begin(115200);

  // avoid chip select contention
  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
 
  tft.begin();
  tft.setRotation(ROTATION);
  tft.fillScreen(ILI9341_BLACK);
  tftWidth = tft.width();
  tftHeight = tft.height();
  ts.begin();
  ts.setRotation(ROTATION);
 
  
  if(!SD.begin(SD_CS)) { 
    Serial.println(F("SD begin() failed again.shit2!"));
    for(;;); // Fatal error, do not continue
  }

  ImageReturnCode stat;
  stat = reader.drawBMP("/test_1.bmp", tft, 0, 0);
  
}

void loop() {
  
}
