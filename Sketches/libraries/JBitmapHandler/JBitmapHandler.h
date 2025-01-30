#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "SD.h"

#ifndef JBitmapHandler_h
#define JBitmapHandler_h

class JBitmapHandler {

public:

  JBitmapHandler();

  void serialPrintHeaders();
  bool renderImage(Adafruit_ILI9341* screen, int screenX, int screenY);
  void loadBitmap(String filename);

private:
  bool readFileHeaders();
  bool checkFileHeaders();
  uint8_t read8Bit();
  uint16_t read16Bit();
  uint32_t read32Bit();

  bool fileOK = false;
  File bmpFile;
  String bmpFilename;

  // BMP header fields
  uint16_t headerField;
  uint32_t fileSize;
  uint32_t imageOffset;
  // DIB header
  uint32_t headerSize;
  uint32_t imageWidth;
  uint32_t imageHeight;
  uint16_t colourPlanes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t imageSize;
  uint32_t xPixelsPerMeter;
  uint32_t yPixelsPerMeter;
  uint32_t totalColors;
  uint32_t importantColors;
};
#endif
