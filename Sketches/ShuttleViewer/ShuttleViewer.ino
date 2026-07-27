#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "JButton.h"
#include "JBitmapHandler.h"
#include "SD.h"

volatile unsigned long displayImageTime = millis();
volatile unsigned long autoFlashTime = millis();
volatile unsigned long doubleClickTimer = 0;
volatile unsigned long debounceTime = millis();

#define stateDebounceDelay 20

#define BUTTON_PIN_PREVIOUS_PICTURE 10  //top button
#define PREVIOUS_X_POS  300
#define PREVIOUS_Y_POS 33

#define BUTTON_PIN_HOMESCREEN 11   //middle button
#define HOMESCREEN_X_POS  300
#define HOMESCREEN_Y_POS  123

#define BUTTON_PIN_NEXT_PICTURE 12 //bottom button
#define NEXT_X_POS  300
#define NEXT_Y_POS  213

#define BUTTON_ICON_SIZE 1

#define INTERRUPT_PIN 7
#define STATUS_LED 2
#define DOUBLE_CLICK_PERIOD 500

#define DISPLAY_INTERVAL 8000

// SPI and TFT pins
#define TFT_CS 5                                          
#define TFT_RST 3 //22
#define TFT_DC 9                       
#define TFT_MOSI 16 //23
#define TFT_CLK 15 //18
#define TFT_MISO 14 //19

// chip select for SD card
 #define SD_CS 13

 bool StartupFailed = false;

#define ROTATION 3

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC/RST
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//XPT2046_Touchscreen ts(TS_CS);

int tftWidth, tftHeight;

// array to hold image filenames
#define MAX_IMAGES 20
String images[MAX_IMAGES];
int imageToShow = 0;
int imageCount = 0;

JBitmapHandler bmh = JBitmapHandler();

volatile JButton button_previous_picture(BUTTON_PIN_PREVIOUS_PICTURE, 0);
volatile JButton button_homescreen(BUTTON_PIN_HOMESCREEN, 1);
volatile JButton button_next_picture(BUTTON_PIN_NEXT_PICTURE, 2);

volatile bool auto_show=false;
volatile bool show_homescreen=false;
volatile bool show_next=false;
volatile bool show_previous=false;
volatile bool secondClick=false;

bool autoFlashOn= false;

void setup() {
  
  Serial.begin(115200);

  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), buttonChangeHandler, CHANGE);

  // avoid chip select contention
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
 
  tft.begin();
  tft.setRotation(ROTATION);
  tft.fillScreen(ILI9341_BLACK);
  tftWidth = tft.width();
  tftHeight = tft.height();

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    StartupFailed=true;
  }
  else {
    // get list of bmp files in root directory 
    File rootFolder = SD.open("/");
    File nextFile;
    String nextFileName;

    // initialise images array
    for (imageCount = 0; imageCount < MAX_IMAGES; imageCount ++) {
      images[imageCount] = String("");
    }

    // load filenames into images array
    imageCount = 0;
    while((nextFile = rootFolder.openNextFile()) && (imageCount < MAX_IMAGES))  {
      // check if directory
      if (!nextFile.isDirectory()) {
        // not a directory - check if bmp
        nextFileName = nextFile.name();
        if (nextFileName.indexOf(".BMP") != -1) {
          // bmp found
          images[imageCount] = String("/" + nextFileName);
          imageCount ++;
        }
      }
      nextFile.close();
    }
    if (nextFile) nextFile.close();
    rootFolder.close(); 

    noInterrupts();
    bmh.loadBitmap("/UFP.BMP");
    bmh.renderImage(&tft,0,0);
    interrupts();
  }

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  Serial.println("SETUP COMPLETE");
}

void printToDisplay(char* pMessage, int16_t foreground_color, int16_t background_color, byte textSize, int16_t xPos, int16_t yPos){
    tft.setTextColor(foreground_color, background_color);
    tft.setTextSize(textSize);
    tft.setCursor(xPos, yPos);
    tft.print(pMessage);
}

void goToHomeScreen(){
  auto_show = false;
  show_next = false;
  show_previous = false;
  show_homescreen = true;
}

void switchToAutoMode(){
  show_homescreen = false;
  show_next = false;
  show_previous = false;
  printToDisplay("AU", ILI9341_GREEN, ILI9341_BLACK, BUTTON_ICON_SIZE, HOMESCREEN_X_POS, HOMESCREEN_Y_POS);
  auto_show = !auto_show;
}

void buttonChangeHandler() {  

  //once detected, act on the first occurance only and ignore any spurious signals for 'stateDebounceDelay' ms
  if ((millis() - debounceTime) > stateDebounceDelay) {
    debounceTime = millis();
  
    if (button_homescreen.onButtonChange()){
      if (doubleClickTimer == 0) { //first click
        doubleClickTimer = millis();
        secondClick = false;         
      }
      else { 
        secondClick=true;
      }
      return;
    }

    if (button_next_picture.onButtonChange()){
      auto_show = false;
      show_homescreen = false;
      show_previous = false;
      show_next = true;
      return;
    }

    if (button_previous_picture.onButtonChange()){
      auto_show = false;
      show_homescreen = false;
      show_previous = true;
      show_next = false;
      return;
    }
  } 
}

void showButtonIcons(byte activeButtonIndex) {
  int16_t buttonOneColor = ILI9341_RED;
  int16_t buttonTwoColor = ILI9341_RED;
  int16_t buttonThreeColor = ILI9341_RED;

  switch (activeButtonIndex) {
    case 1:
      buttonOneColor = ILI9341_GREEN;
      break;
    case 2:
      buttonTwoColor = ILI9341_GREEN;
      break;
    case 3:
      buttonThreeColor = ILI9341_GREEN;
      break;
    default:
      break;
  }
  printToDisplay("<<", buttonOneColor, ILI9341_BLACK, BUTTON_ICON_SIZE, PREVIOUS_X_POS, PREVIOUS_Y_POS);
  printToDisplay("AU", buttonTwoColor, ILI9341_BLACK, BUTTON_ICON_SIZE, HOMESCREEN_X_POS, HOMESCREEN_Y_POS);
  printToDisplay(">>", buttonThreeColor, ILI9341_BLACK, BUTTON_ICON_SIZE, NEXT_X_POS, NEXT_Y_POS);
}

void loop() {
  
  if (!StartupFailed) {

    if (doubleClickTimer > 0) { //timer is set so a click evenet occurred.  Wait until the timer is complete and then execute either method
      while (millis()<(doubleClickTimer + DOUBLE_CLICK_PERIOD)) {
      }

      if (secondClick) {
        goToHomeScreen();
      }
      else {
        switchToAutoMode();
      }

      doubleClickTimer = 0;
    }

    secondClick=false;

    if (show_homescreen==true ){
      tft.fillScreen(ILI9341_BLACK);
      noInterrupts();
      bmh.loadBitmap("/UFP.BMP");
      bmh.renderImage(&tft,0,0);
      show_homescreen = false;
      interrupts();
    }

    if (show_previous==true){
      noInterrupts();
      imageToShow--;
      
      if (imageToShow < 1){
         imageToShow = (imageCount-1);
      } 

      if (images[imageToShow] != "") {
        tft.fillScreen(ILI9341_BLACK);
        bmh.loadBitmap(images[imageToShow]);
        bmh.renderImage(&tft,0,0);
        showButtonIcons(1);
      }
      show_previous = false;
      interrupts();
    }

    if (show_next==true){
      noInterrupts();
      imageToShow ++;
      
      if (imageToShow > (imageCount-1)){
         imageToShow = 0;
      } 

      if (images[imageToShow] != "") {
        tft.fillScreen(ILI9341_BLACK);
        bmh.loadBitmap(images[imageToShow]);
        bmh.renderImage(&tft,0,0);
        showButtonIcons(3);
      }
      show_next = false;
      interrupts();
    }

    if (auto_show==true) { //and timer is cleared or expired

      if (millis() > (autoFlashTime + 800)){ //alternate the color of the button icon
        autoFlashTime = millis();
        if (autoFlashOn) {
          printToDisplay("AU", ILI9341_DARKGREEN, ILI9341_BLACK, BUTTON_ICON_SIZE, HOMESCREEN_X_POS, HOMESCREEN_Y_POS);
        }
        else {

          printToDisplay("AU", ILI9341_GREEN, ILI9341_BLACK, BUTTON_ICON_SIZE, HOMESCREEN_X_POS, HOMESCREEN_Y_POS);
        }
        autoFlashOn = !autoFlashOn;
      }

      if (millis() > (displayImageTime + DISPLAY_INTERVAL) ||  //sh0w the next image
          (displayImageTime == 0)){

        noInterrupts();

        if (images[imageToShow] != "") {
          tft.fillScreen(ILI9341_BLACK);
          bmh.loadBitmap(images[imageToShow]);
          bmh.renderImage(&tft,0,0);
          showButtonIcons(2);
          displayImageTime=millis();
        }

        imageToShow ++;
        if (imageToShow >= MAX_IMAGES){
          imageToShow = 1;
        } 

        interrupts();
      } //end rendering
    } //if auto_show is on
  }//if not failed
}
