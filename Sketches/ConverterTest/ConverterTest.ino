#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "JBitmapHandler.h"
//#include "XPT2046_Touchscreen.h"
#include "SD.h"
#include "Arduino.h"

// SPI and TFT pins
#define TFT_CS 5     //7
#define TFT_RST 3    //6
#define TFT_DC 7     //5
#define TFT_MOSI 16  //4
#define TFT_CLK 15   //3
#define TFT_MISO 14  //1
#define SD_CS 10

void setup() {
  
  Serial.begin(115200);

  pinMode(TFT_CS, OUTPUT); 
  pinMode(TFT_RST, OUTPUT); 
  pinMode(TFT_DC, OUTPUT); 
  pinMode(TFT_MOSI, OUTPUT); 
  pinMode(TFT_CLK, OUTPUT); 
  pinMode(TFT_MISO, OUTPUT); 
  pinMode(SD_CS, OUTPUT);
}

void loop() {
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TFT_RST, HIGH);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_MOSI, HIGH);
  digitalWrite(TFT_CLK, HIGH);
  digitalWrite(TFT_MISO, HIGH);
  digitalWrite(SD_CS, HIGH);

  delay(5000);

  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RST, LOW);
  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_MOSI, LOW);
  digitalWrite(TFT_CLK, LOW);
  digitalWrite(TFT_MISO, LOW);
  digitalWrite(SD_CS, LOW);

  delay(5000); 

  // put your main code here, to run repeatedly:

}
