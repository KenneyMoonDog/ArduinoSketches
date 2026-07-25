#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "JButton.h"
#include "JBitmapHandler.h"
#include "SD.h"

volatile unsigned long displayImageTime = millis();
volatile unsigned long currentMillis = millis();
volatile unsigned long doubleClickTimer = 0;

unsigned long stateDebounceDelay = 20;
unsigned long debounceTime = millis();

//defines for video control buttons
#define BUTTON_PIN_HOMESCREEN 10
#define BUTTON_PIN_AUTO_SHOW 11
#define BUTTON_PIN_NEXT_PICTURE 12
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

volatile JButton button_homescreen(BUTTON_PIN_HOMESCREEN, 0);
volatile JButton button_auto_show(BUTTON_PIN_AUTO_SHOW, 1);
volatile JButton button_next_picture(BUTTON_PIN_NEXT_PICTURE, 2);

volatile bool auto_show=false;
volatile bool show_homescreen=false;
volatile bool show_next=false;
volatile bool show_previous=false;

volatile bool homescreenSingleClick=false;
volatile bool homescreenDoubleClick=false;

void setup() {
  
  Serial.begin(115200);
  //Serial.begin(9600);

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

void printToDisplay(char* pMessage) {
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print(pMessage);
}

void printToDisplay(char* pMessage, int16_t background_color){
    tft.setTextColor(ILI9341_RED, background_color);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
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
  auto_show = true;
}

void buttonChangeHandler() {  

  //once detected, act on the first occurance only and ignore any spurious signals for 'stateDebounceDelay' ms
  if ((millis() - debounceTime) > stateDebounceDelay) {
    debounceTime = millis();
  
    if (button_homescreen.onButtonChange()){

      if (homescreenSingleClick==true) { //this may be a double click if within a certain time
        if (millis() < (doubleClickTimer + DOUBLE_CLICK_PERIOD)) {  //then you have a double click on this button and set mode to AUTO
          homescreenSingleClick = false;
          homescreenDoubleClick = true;
          doubleClickTimer = 0;
          return;
        }  
        else {  //doubleClick has timed out.. Treat as a single click
          homescreenSingleClick = true;
          homescreenDoubleClick = false;
          doubleClickTimer = 0;
          return;
        }
      }
      else { //first time click
        doubleClickTimer = millis();
        homescreenSingleClick = true;
        homescreenDoubleClick = false;

        //goToHomeScreen();
        //auto_show = false;
        //show_next = false;
        //show_previous = false;
        //show_homescreen = true;*/
        return;
      }
    }

    if (button_auto_show.onButtonChange()){
      show_homescreen = false;
      show_next = false;
      show_previous = false;
      auto_show = !auto_show;

      if (auto_show == false) {
        printToDisplay("Auto Display OFF", ILI9341_BLACK);
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
  } 
}

void loop() {
  
  if (!StartupFailed) {

    if (homescreenDoubleClick) {
      switchToAutoMode();
      homescreenDoubleClick = false;
    }
    else if( homescreenSingleClick ) {
      if (millis() > (doubleClickTimer + DOUBLE_CLICK_PERIOD)) {
        goToHomeScreen();
        homescreenSingleClick = false;
      }
    }

    if (show_homescreen==true ){
      tft.fillScreen(ILI9341_BLACK);
      printToDisplay("Returning to Homescreen...", ILI9341_BLACK);
      delay(2000);

      noInterrupts();
      bmh.loadBitmap("/UFP.BMP");
      bmh.renderImage(&tft,0,0);
      show_homescreen = false;
      interrupts();
      
      return;
    }

    if (show_previous==true){
      noInterrupts();
      imageToShow--;
      
      if (imageToShow < 1){
         imageToShow = (imageCount-1);
      } 

      if (images[imageToShow] != "") {
        tft.fillScreen(ILI9341_BLACK);
        printToDisplay("Show Previous...");
        bmh.loadBitmap(images[imageToShow]);
        bmh.renderImage(&tft,0,0);
        printToDisplay("Show Previous...");
      }
      show_previous = false;
      interrupts();
      return;
    }

    if (show_next==true){
      noInterrupts();
      imageToShow ++;
      
      if (imageToShow > (imageCount-1)){
         imageToShow = 0;
      } 

      if (images[imageToShow] != "") {
        tft.fillScreen(ILI9341_BLACK);
        printToDisplay("Show Next...");
        bmh.loadBitmap(images[imageToShow]);
        bmh.renderImage(&tft,0,0);
        printToDisplay("Show Next...");
      }
      show_next = false;
      interrupts();
      return;
    }

    if (auto_show==true) { //and timer is cleared or expired
      currentMillis = millis();

      if (currentMillis > (displayImageTime + DISPLAY_INTERVAL) ||
          (displayImageTime == 0)){

        noInterrupts();

        if (images[imageToShow] != "") {
          tft.fillScreen(ILI9341_BLACK);
          printToDisplay("Auto Display ON ");
          bmh.loadBitmap(images[imageToShow]);
          bmh.renderImage(&tft,0,0);
          printToDisplay("Auto Display ON ");
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
