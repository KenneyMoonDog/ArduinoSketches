
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "JBitmapHandler.h"
//#include "XPT2046_Touchscreen.h"
#include "SD.h"

// SPI and TFT pins
#define TFT_CS A5
#define TFT_DC A3
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST A4
#define TFT_MISO 12

// Chip select for touch panel
//#define TS_CS 7

// chip select for SD card
#define SD_CS A2

#define ROTATION 3

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC/RST
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//XPT2046_Touchscreen ts(TS_CS);

int tftWidth, tftHeight;

// array to hold image filenames
#define MAX_IMAGES 20
String images[MAX_IMAGES];
int imageToShow = 0;

void setup() {
  
  Serial.begin(115200);

  // avoid chip select contention
//  pinMode(TS_CS, OUTPUT);
//  digitalWrite(TS_CS, HIGH);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
 
  tft.begin();
  tft.setRotation(ROTATION);
  tft.fillScreen(ILI9341_BLACK);
  tftWidth = tft.width();
  tftHeight = tft.height();
  //ts.begin();
  //ts.setRotation(ROTATION);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
  }
  
  // get list of bmp files in root directory
  File rootFolder = SD.open("/");
  File nextFile;
  String nextFileName;
  int imageCount = 0;

  // initialise images array
  for (imageCount = 0; imageCount <= MAX_IMAGES; imageCount ++) {
    images[imageCount] = String("");
  }

  // load filenames into images array
  imageCount = 0;
  while((nextFile = rootFolder.openNextFile()) && (imageCount < MAX_IMAGES))  {
    // check if directory
    if (!nextFile.isDirectory()) {
      // not a directory - check if bmp
      nextFileName = nextFile.name();
      Serial.print(nextFileName);
      Serial.println(" found");
      if (nextFileName.indexOf(".BMP") != -1) {
        // bmp found
        Serial.println("Bitmap");
        images[imageCount] = String(nextFileName);
        imageCount ++;
      }
    }
    nextFile.close();
  }
  if (nextFile) nextFile.close();
  rootFolder.close(); 
}

void loop() {

  Serial.print("Showing image ");
  Serial.print(imageToShow);
  Serial.print(" - ");
  Serial.println(images[imageToShow]);
  
  if (images[imageToShow] != "") {
    Serial.println("showing");
    JBitmapHandler bmh = JBitmapHandler(images[imageToShow]);
    //bmh.serialPrintHeaders();
    tft.fillScreen(ILI9341_DARKGREY);
    bmh.renderImage(tft,0,0);
    delay(2000);
  }
  imageToShow ++;
  if (imageToShow >= MAX_IMAGES){
    imageToShow = 0;
  }

}
