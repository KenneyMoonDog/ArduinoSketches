
#include "JBitmapHandler.h"

bool fileOK = false;
File bmpFile;
String bmpFilename;

// BMP header fields
uint16_t headerField = 0;
uint32_t fileSize = 0;
uint32_t imageOffset = 0;
  // DIB header
uint32_t headerSize = 0;
uint32_t imageWidth = 0;
uint32_t imageHeight = 0;
uint16_t colourPlanes = 0;
uint16_t bitsPerPixel = 0;
uint32_t compression = 0;
uint32_t imageSize = 0;
uint32_t xPixelsPerMeter = 0;
uint32_t yPixelsPerMeter = 0;
uint32_t totalColors = 0;
uint32_t importantColors = 0;

  JBitmapHandler::JBitmapHandler(String filename){
    fileOK = false;
    bmpFilename = filename;

    bmpFile = SD.open(bmpFilename, FILE_READ);
    if (!bmpFile) {
      Serial.print(F("BitmapHandler : Unable to open file "));
      Serial.println(bmpFilename);
      fileOK = false;
    }
    else {
      if (!readFileHeaders()){
        Serial.println(F("Unable to read file headers"));
        fileOK = false;
      }
      else {
        if (!checkFileHeaders()){
          Serial.println(F("Not compatible file"));
          fileOK = false;
        }
        else {
          // all OK
          Serial.println(F("BMP file all OK"));
          fileOK = true;
        }
      }
      // close file
      bmpFile.close();
    }
  }

  uint8_t JBitmapHandler::read8Bit(){
      if (!bmpFile) {
       return 0;
      }
      else {
        return bmpFile.read();
      }
    }

    uint16_t JBitmapHandler::read16Bit(){
      uint16_t lsb, msb;
      if (!bmpFile) {
       return 0;
      }
      else {
        lsb = bmpFile.read();
        msb = bmpFile.read();
        return (msb<<8) + lsb;
      }
    }

    uint32_t JBitmapHandler::read32Bit(){
      uint32_t lsb, b2, b3, msb;
      if (!bmpFile) {
       return 0;
      }
      else {
        lsb = bmpFile.read();
        b2 = bmpFile.read();
        b3 = bmpFile.read();
        msb = bmpFile.read();
        return (msb<<24) + (b3<<16) + (b2<<8) + lsb;
      }
    }

    bool JBitmapHandler::readFileHeaders(){
      if (bmpFile) {
        // reset to start of file
        bmpFile.seek(0);

        // BMP Header
        headerField = read16Bit();
        fileSize = read32Bit();
        read16Bit(); // reserved
        read16Bit(); // reserved
        imageOffset = read32Bit();

        // DIB Header
        headerSize = read32Bit();
        imageWidth = read32Bit();
        imageHeight = read32Bit();
        colourPlanes = read16Bit();
        bitsPerPixel = read16Bit();
        compression = read32Bit();
        imageSize = read32Bit();
        xPixelsPerMeter = read32Bit();
        yPixelsPerMeter = read32Bit();
        totalColors = read32Bit();
        importantColors = read32Bit();

        bmpFile.close();
        // close file
        return true;
      }
      else {
        return false;
      }
    }

    bool JBitmapHandler::checkFileHeaders(){

      // BMP file id
      if (headerField != 0x4D42){
        return false;
      }
      // must be single colour plane
      if (colourPlanes != 1){
        return false;
      }
      // only working with 24 bit bitmaps
      if (bitsPerPixel != 24){
        return false;
      }
      // no compression
      if (compression != 0){
        return false;
      }
      // all ok
      return true;
    }

    void JBitmapHandler::serialPrintHeaders() {
      Serial.print(F("filename : "));
      Serial.println(bmpFilename);
      // BMP Header
      Serial.print(F("headerField : "));
      Serial.println(headerField, HEX);
      Serial.print(F("fileSize : "));
      Serial.println(fileSize);
      Serial.print(F("imageOffset : "));
      Serial.println(imageOffset);
      Serial.print(F("headerSize : "));
      Serial.println(headerSize);
      Serial.print(F("imageWidth : "));
      Serial.println(imageWidth);
      Serial.print(F("imageHeight : "));
      Serial.println(imageHeight);
      Serial.print(F("colourPlanes : "));
      Serial.println(colourPlanes);
      Serial.print(F("bitsPerPixel : "));
      Serial.println(bitsPerPixel);
      Serial.print(F("compression : "));
      Serial.println(compression);
      Serial.print(F("imageSize : "));
      Serial.println(imageSize);
      Serial.print(F("xPixelsPerMeter : "));
      Serial.println(xPixelsPerMeter);
      Serial.print(F("yPixelsPerMeter : "));
      Serial.println(yPixelsPerMeter);
      Serial.print(F("totalColors : "));
      Serial.println(totalColors);
      Serial.print(F("importantColors : "));
      Serial.println(importantColors);
    }

    void JBitmapHandler::renderImage(Adafruit_ILI9341 screen, int screenX, int screenY){

      // read from sd card in blocks
      uint8_t pixelBuffer[3 * 10];
      uint8_t pixelBufferCounter = sizeof(pixelBuffer);

      int bytesPerRow;
      int displayedWidth, displayedHeight;
      int pixelRow, pixelCol;
      uint32_t pixelRowFileOffset;
      uint8_t r,g,b;

      // check if file OK
      if (!fileOK) {
        return false;
      }

      // make sure screenX, screenY is on screen
      if((screenX < 0) || (screenX >= screen.width())
        || (screenY < 0) || (screenY >= screen.height())){
        return false;
      }

      // get dimensions of displayed image - crop if needed
      displayedWidth = imageWidth;
      displayedHeight = imageHeight;
      if (displayedWidth > (screen.width() - screenX)){
        displayedWidth = screen.width() - screenX;
      }
      if (displayedHeight > (screen.height() - screenY)){
        displayedHeight = screen.height() - screenY;
      }

      // bytes per row rounded up to next 4 byte boundary
      bytesPerRow = (3 * imageWidth) + ((4 - ((3 * imageWidth) % 4)) % 4);
      screen.endWrite();
      // open file
      //char localFilename[bmpFilename.length() + 1];
      //bmpFilename.toCharArray(localFilename,bmpFilename.length());
      Serial.println("-----------------------------------");
      Serial.println(bmpFilename);
      Serial.println("-----------------------------------");
      bmpFile = SD.open(bmpFilename, FILE_READ);
      Serial.println("-----------------------------------");
      Serial.println(bmpFilename);
      Serial.println("-----------------------------------");

       //set up tft byte write area
      screen.startWrite();
      screen.setAddrWindow(screenX, screenY, displayedWidth, displayedHeight);
      screen.endWrite();
      Serial.print(F("screenX : "));
      Serial.println(screenX);
      Serial.print(F("screenY : "));
      Serial.println(screenY);
      Serial.print(F("displayedWidth : "));
      Serial.println(displayedWidth);
      Serial.print(F("displayedHeight : "));
      Serial.println(displayedHeight);


      for (pixelRow = 0; pixelRow < displayedHeight; pixelRow ++) {
        // image stored bottom to top, screen top to bottom
        pixelRowFileOffset = imageOffset + ((imageHeight - pixelRow - 1) * bytesPerRow);

        // move file pointer to start of row pixel data
        screen.endWrite(); // turn off pixel stream while acxcessing sd card
        bmpFile.seek(pixelRowFileOffset);

        // reset buffer
        pixelBufferCounter = sizeof(pixelBuffer);

        // output pixels in row
        for (pixelCol = 0; pixelCol < displayedWidth; pixelCol ++) {
          if (pixelBufferCounter >= sizeof(pixelBuffer)){
            // need to read more from sd card
            screen.endWrite(); // turn of pixel stream while acxcessing sd card
            bmpFile.read(pixelBuffer, sizeof(pixelBuffer));
            pixelBufferCounter = 0;
            screen.startWrite(); // turn on pixel stream
          }

          // get next pixel colours
          b = pixelBuffer[pixelBufferCounter++];
          g = pixelBuffer[pixelBufferCounter++];
          r = pixelBuffer[pixelBufferCounter++];

          // send pixel to tft
          screen.writeColor(screen.color565(r,g,b),1);
        } // pixelCol
      } // pixelRow */
      screen.endWrite(); // turn off pixel stream
      bmpFile.close();
}

JBitmapHandler::JBitmapHandler() {
}

/*bool JBitmapHandler::openAndRead(const char* filename){
  fileOK = false;
  bmpFilename = String(filename);
  bmpFile = SD.open(bmpFilename, FILE_READ);
  if (!bmpFile) {
    Serial.print(F("BitmapHandler : Unable to open file "));
    Serial.println(bmpFilename);
    fileOK = false;
  }
  else {
    if (!readFileHeaders()){
      Serial.println(F("Unable to read file headers"));
      fileOK = false;
    }
    else {
      if (!checkFileHeaders()){
        Serial.println(F("Not compatible file"));
        fileOK = false;
      }
      else {
        // all OK
        Serial.println(F("BMP file all OK"));
        fileOK = true;
      }
    }
    // close file
    bmpFile.close();
  }

  return fileOK;
}*/
