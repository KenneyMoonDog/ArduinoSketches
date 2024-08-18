
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "JBitmapHandler.h"
//#include "XPT2046_Touchscreen.h"
#include "SD.h"
#include "string.h"

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

JBitmapHandler mBitmapHandler = JBitmapHandler();
int imageIndex = 0;

// array to hold image filenames
#define MAX_IMAGES 20
#define MAX_NAME_LENGTH 30
char images[MAX_IMAGES][MAX_NAME_LENGTH];
char nextFileName[MAX_NAME_LENGTH] = ("              ");

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
  //tft.fillScreen(ILI9341_BLACK);
  tftWidth = tft.width();
  tftHeight = tft.height();
  //ts.begin();
  //ts.setRotation(ROTATION);

  if (!SD.begin(SD_CS)) {
  }

  // load filenames into images array
  imageIndex = 0;
  while((nextFile = rootFolder.openNextFile()) && (imageIndex < MAX_IMAGES))  {
    // check if directory
    if (!nextFile.isDirectory()) {
      // not a directory - check if bmp

      byte strLength = strlen(nextFile.name()) + 1;
      Serial.println(" FOUND:");
      Serial.println(nextFile.name());
      if (strLength < MAX_NAME_LENGTH) {
        strcpy(nextFileName, nextFile.name());
        Serial.println(nextFileName);
      }
     // else {
     //   Serial.println(strLength);
     // }
/*
          int strLength = images[imageToShow].length() + 1;
    char fileName[strLength];
    const char* pFileName= &fileName[0];
    images[imageToShow].toCharArray(fileName, strLength);
*/    
      if (strrchr(nextFileName,".BMP")) {
     // if (nextFileName.indexOf(".BMP") != -1) {
        // bmp found
        Serial.println("Bitmap");
        strcpy(images[imageIndex], nextFileName);
        imageIndex++;
      }
    }
    nextFile.close();
  }

  if (nextFile) nextFile.close();
  rootFolder.close(); 

  Serial.println("IMAGE DUMP:");
  for (byte imgCount = 0; imgCount < MAX_IMAGES; imgCount++){
    if (images[imgCount] != NULL){
      Serial.println(images[imgCount]);
    }
  }
  imageIndex = 0;
}

void loop() {

  while(1){};

  Serial.print("Showing image ");
  Serial.print(imageIndex);
  Serial.print(" - ");
  Serial.println(images[imageIndex]);
  
  if (images[imageIndex] != NULL) {

    int strLength =  strlen(images[imageIndex]) + 1;
    char fileName[strLength];
    const char* pFileName= &fileName[0];
    strcpy(fileName[imageIndex], fileName);

    mBitmapHandler.openAndRead(pFileName);
    mBitmapHandler.serialPrintHeaders();
    tft.fillScreen(ILI9341_DARKGREY);
    mBitmapHandler.renderImage(tft,0,0);
    mBitmapHandler.resetHandler();
    delay(2000);
  }

  imageIndex ++;
  if (imageIndex >= MAX_IMAGES){
    imageIndex = 0;
  }

}

    Serial.println("SD Card initialization failed!");
  }
  
  // get list of bmp files in root directory
  File rootFolder = SD.open("/");
  File nextFile;

  // initialise images array
  //char emptyString[10] = "          ";
 // for (imageIndex = 0; imageIndex <= MAX_IMAGES; imageIndex ++) {
 //   strcpy(images[imageIndex],"\0");