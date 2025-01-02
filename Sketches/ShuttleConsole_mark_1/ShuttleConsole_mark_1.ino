#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
//#include "XPT2046_Touchscreen.h"
#include "SD.h"
#include "JBitmapHandler.h"

// SPI and TFT pins
#define TFT_CS A5
#define TFT_DC A3
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST A4
#define TFT_MISO 12


// Chip select for touch panel
//#define TS_CS 7

// chip select for SD c#ard
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

class BitmapHandler {

  private:
    bool fileOK = false;
    File bmpFile;
    String bmpFilename;

    uint8_t read8Bit(){
      if (!this->bmpFile) {
       return 0;
      }
      else {
        return this->bmpFile.read();
      }
    }

    uint16_t read16Bit(){
      uint16_t lsb, msb;
      if (!this->bmpFile) {
       return 0;
      }
      else {
        lsb = this->bmpFile.read();
        msb = this->bmpFile.read();
        return (msb<<8) + lsb;
      }
    }

    uint32_t read32Bit(){
      uint32_t lsb, b2, b3, msb;
      if (!this->bmpFile) {
       return 0;
      }
      else {
        lsb = this->bmpFile.read();
        b2 = this->bmpFile.read();
        b3 = this->bmpFile.read();
        msb = this->bmpFile.read();
        return (msb<<24) + (b3<<16) + (b2<<8) + lsb;
      }
    }
    
  public:

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

    BitmapHandler(String filename){
      this->fileOK = false;
      this->bmpFilename = filename;
      this->bmpFile = SD.open(this->bmpFilename, FILE_READ);
      if (!this->bmpFile) {
        Serial.print(F("BitmapHandler : Unable to open file "));
        Serial.println(this->bmpFilename);
        this->fileOK = false;
      }
      else {
        if (!this->readFileHeaders()){
          Serial.println(F("Unable to read file headers"));
          this->fileOK = false;
        }
        else {
          if (!this->checkFileHeaders()){
            Serial.println(F("Not compatible file"));
            this->fileOK = false;
          }
          else {
            // all OK
            Serial.println(F("BMP file all OK"));
            this->fileOK = true;
          }       
        }
        // close file
        this->bmpFile.close();
      }
    }
    
    bool readFileHeaders(){
      if (this->bmpFile) {
        // reset to start of file
        this->bmpFile.seek(0);
        
        // BMP Header
        this->headerField = this->read16Bit();
        this->fileSize = this->read32Bit();
        this->read16Bit(); // reserved
        this->read16Bit(); // reserved
        this->imageOffset = this->read32Bit();

        // DIB Header
        this->headerSize = this->read32Bit();
        this->imageWidth = this->read32Bit();
        this->imageHeight = this->read32Bit();
        this->colourPlanes = this->read16Bit();
        this->bitsPerPixel = this->read16Bit();
        this->compression = this->read32Bit();
        this->imageSize = this->read32Bit();
        this->xPixelsPerMeter = this->read32Bit();
        this->yPixelsPerMeter = this->read32Bit();
        this->totalColors = this->read32Bit();
        this->importantColors = this->read32Bit();

        // close file
        return true;
      }
      else {
        return false;
      }
    }

    bool checkFileHeaders(){

      // BMP file id
      if (this->headerField != 0x4D42){
        return false;
      }
      // must be single colour plane
      if (this->colourPlanes != 1){
        return false;
      }
      // only working with 24 bit bitmaps
      if (this->bitsPerPixel != 24){
        return false;
      }
      // no compression
      if (this->compression != 0){
        return false;
      }
      // all ok
      return true;
    }

    void serialPrintHeaders() {
      Serial.print(F("filename : "));
      Serial.println(this->bmpFilename);
      // BMP Header
      Serial.print(F("headerField : "));
      Serial.println(this->headerField, HEX);
      Serial.print(F("fileSize : "));
      Serial.println(this->fileSize);
      Serial.print(F("imageOffset : "));
      Serial.println(this->imageOffset);
      Serial.print(F("headerSize : "));
      Serial.println(this->headerSize);
      Serial.print(F("imageWidth : "));
      Serial.println(this->imageWidth);
      Serial.print(F("imageHeight : "));
      Serial.println(this->imageHeight);
      Serial.print(F("colourPlanes : "));
      Serial.println(this->colourPlanes);
      Serial.print(F("bitsPerPixel : "));
      Serial.println(this->bitsPerPixel);
      Serial.print(F("compression : "));
      Serial.println(this->compression);
      Serial.print(F("imageSize : "));
      Serial.println(this->imageSize);
      Serial.print(F("xPixelsPerMeter : "));
      Serial.println(this->xPixelsPerMeter);
      Serial.print(F("yPixelsPerMeter : "));
      Serial.println(this->yPixelsPerMeter);
      Serial.print(F("totalColors : "));
      Serial.println(this->totalColors);
      Serial.print(F("importantColors : "));
      Serial.println(this->importantColors);
    }

    void renderImage(Adafruit_ILI9341 screen, int screenX, int screenY){
      
      // read from sd card in blocks
      uint8_t pixelBuffer[3 * 10];
      uint8_t pixelBufferCounter = sizeof(pixelBuffer);
      
      int bytesPerRow;
      int displayedWidth, displayedHeight;
      int pixelRow, pixelCol;
      uint32_t pixelRowFileOffset;
      uint8_t r,g,b;

      // check if file OK
      if (!this->fileOK) {
        return false;
      }

      // make sure screenX, screenY is on screen
      if((screenX < 0) || (screenX >= screen.width()) 
        || (screenY < 0) || (screenY >= screen.height())){
        return false;
      }

      // get dimensions of displayed image - crop if needed
      displayedWidth = this->imageWidth;
      displayedHeight = this->imageHeight;
      if (displayedWidth > (screen.width() - screenX)){
        displayedWidth = screen.width() - screenX;
      }
      if (displayedHeight > (screen.height() - screenY)){
        displayedHeight = screen.height() - screenY;
      }

      // bytes per row rounded up to next 4 byte boundary
      bytesPerRow = (3 * this->imageWidth) + ((4 - ((3 * this->imageWidth) % 4)) % 4);

      // open file
      this->bmpFile = SD.open(this->bmpFilename, FILE_READ);

      // set up tft byte write area
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
        pixelRowFileOffset = this->imageOffset + ((this->imageHeight - pixelRow - 1) * bytesPerRow);

        // move file pointer to start of row pixel data
        screen.endWrite(); // turn off pixel stream while acxcessing sd card
        this->bmpFile.seek(pixelRowFileOffset);

        // reset buffer
        pixelBufferCounter = sizeof(pixelBuffer);

        // output pixels in row
        for (pixelCol = 0; pixelCol < displayedWidth; pixelCol ++) {
          if (pixelBufferCounter >= sizeof(pixelBuffer)){
            // need to read more from sd card
            screen.endWrite(); // turn of pixel stream while acxcessing sd card
            this->bmpFile.read(pixelBuffer, sizeof(pixelBuffer));
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
        
      } // pixelRow
      screen.endWrite(); // turn off pixel stream
      this->bmpFile.close();
    }
};

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
    bmh.serialPrintHeaders();
    tft.fillScreen(ILI9341_DARKGREY);
    bmh.renderImage(tft,0,0);
    delay(2000);
  }
  imageToShow ++;
  if (imageToShow >= MAX_IMAGES){
    imageToShow = 0;
  }

}