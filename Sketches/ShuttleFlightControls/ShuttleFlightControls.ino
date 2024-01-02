#include "Arduino.h"

#include <Adafruit_NeoPixel.h>


unsigned long pixelOnePeriod = 2000; //msec
unsigned long pixelTwoPeriod = 3000;
unsigned long pixelThreePeriod = 1500;
unsigned long pixelFourPeriod = 2700;

unsigned long lastPixelOnePeriod = 0; //msec
unsigned long lastPixelTwoPeriod = 0;
unsigned long lastPixelThreePeriod = 0;
unsigned long lastPixelFourPeriod = 0;

unsigned long previousMillis = 0;

#define DELAY_PERIOD 100
#define PIN_UPPER_PANEL_SET_1 0
#define PIN_UPPER_PANEL_SET_2 2   //not PWM
#define PIN_SCANNER 3

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN   1
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 4

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

const long red = 65535;
const long pink = 65536*5/6;
const long blue = 65536*2/3;
const long teal = 65536/2;
const long green = 65536/3;
const long yellow = 65536/6;

long colorSet[LED_COUNT] = {red, pink, blue, green};
long hue[LED_COUNT] = {blue, blue, blue, blue}; //0 (red) to 65535 (red)
byte sat[LED_COUNT] = {50, 255, 100, 230}; //(0-255) grey to full color 
byte bright[LED_COUNT] = {25, 40, 20, 60}; //(0-255) black to full bright
byte inout[LED_COUNT] = {1,0,1,0}; //0 or 1 
byte satStep[LED_COUNT] =  {1,1,1,1};
byte brightStep[LED_COUNT] = {1,2,1,2};
byte maxBright = 180;
byte minBright = 1;

void setup() {
  pinMode(PIN_UPPER_PANEL_SET_1, OUTPUT);
  pinMode(PIN_UPPER_PANEL_SET_2, OUTPUT);
  pinMode(PIN_SCANNER, OUTPUT);
  digitalWrite(PIN_UPPER_PANEL_SET_1, LOW);
  digitalWrite(PIN_UPPER_PANEL_SET_2, LOW);
  digitalWrite(PIN_SCANNER, LOW);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
}

void timedActivities(){
  unsigned long currentMillis = millis();

  if (currentMillis - lastPixelOnePeriod >= pixelOnePeriod) { //execute any timed operations every DELAY_PERIOD ms
    lastPixelOnePeriod = currentMillis;
    //reset the period?
    //checkNacelleLevel();
  }
  
  if (currentMillis - lastPixelTwoPeriod >= pixelTwoPeriod) { //execute any timed operations every DELAY_PERIOD ms
    lastPixelTwoPeriod = currentMillis;
    //reset the period?
    //checkNacelleLevel();
  }
  
  if (currentMillis - lastPixelThreePeriod >= pixelThreePeriod) { //execute any timed operations every DELAY_PERIOD ms
    lastPixelThreePeriod = currentMillis;
    //reset the period?
    //checkNacelleLevel();
  }
  
   if (currentMillis - lastPixelFourPeriod >= pixelFourPeriod) { //execute any timed operations every DELAY_PERIOD ms
    lastPixelFourPeriod = currentMillis;
    //reset the period?
    //checkNacelleLevel();
  }
}



void loop() {
  delay(DELAY_PERIOD);
  timedActivities();
}
