#include <SERIAL_COMM_MK2.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

volatile unsigned long previousMillis = 0;
volatile unsigned long previousEngineMillis = 0;

int incomingByte = 0;   // for incoming serial data
byte stateIn = 0;
byte newNacelleRGB[] = {10,10,10};
byte oldNacelleRGB[] = {0,0,0};

boolean bWarpOn = false;
boolean enginePauseOn = false;
boolean bPowerOn = false;

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 10 //ms

#define PIN_NAVIGATION_FLASHER 15
#define PIN_BEACON_FLASHER 16
#define PIN_ENGINE_FLOOD 8
#define PIN_FORWARD_FLOOD 3
#define PIN_NACELLE_B 9
#define PIN_NACELLE_G 10
#define PIN_NACELLE_R 11

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN   17
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 6

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

long colorSet[LED_COUNT] = {red, pink, blue, teal, green, yellow};
long hue[LED_COUNT] = {blue, blue, blue, blue, blue, blue}; //0 (red) to 65535 (red)
byte sat[LED_COUNT] = {50, 255, 100, 230, 20, 175}; //(0-255) grey to full color 
byte bright[LED_COUNT] = {25, 40, 20, 60, 10, 50}; //(0-255) black to full bright
byte inout[LED_COUNT] = {1,0,1,0,1,0}; //0 or 1 
byte satStep[LED_COUNT] =  {1,1,1,1,1,1};
byte brightStep[LED_COUNT] = {1,2,1,2,1,2};
byte maxBright = 180;
byte minBright = 1;

void setup() {    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_BEACON_FLASHER, OUTPUT);
  pinMode(PIN_ENGINE_FLOOD, OUTPUT);   
  pinMode(PIN_FORWARD_FLOOD, OUTPUT);
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
  digitalWrite(PIN_BEACON_FLASHER, LOW);

  pinMode(PIN_NACELLE_R, OUTPUT);
  pinMode(PIN_NACELLE_G, OUTPUT);
  pinMode(PIN_NACELLE_B, OUTPUT);
  
  analogWrite(PIN_NACELLE_R, 255);
  analogWrite(PIN_NACELLE_G, 255);
  analogWrite(PIN_NACELLE_B, 255);
  //resetWarpEngine();
  
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

    // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
    // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  //strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) { //execute any timed operations every INTERRRUPT FREQ ms
    previousMillis = currentMillis;
    checkNacelleLevel();
  }

  /*if (currentMillis - redAlertPreviousMillis >= RECEIVER_INTERRUPT_FREQ_RED_ALERT) {
    redAlertPreviousMillis = currentMillis;
    updateRedAlert();
  }*/

  /* warp_start();
  warp_cruise_4();
  warp_decell_2();
   */
} 

void setNacelle(byte color[]) {
      newNacelleRGB[0] = color[0];
      newNacelleRGB[1] = color[1];
      newNacelleRGB[2] = color[2];
}

void checkNacelleLevel() {

    bool oper_R = (oldNacelleRGB[0] >= newNacelleRGB[0]) ? 0:1;
    bool oper_G = (oldNacelleRGB[1] >= newNacelleRGB[1]) ? 0:1;
    bool oper_B = (oldNacelleRGB[2] >= newNacelleRGB[2]) ? 0:1;
  
     if ( oldNacelleRGB[0] != newNacelleRGB[0] ) {
        if ( oper_R ) {
          analogWrite(PIN_NACELLE_R, (255-(++oldNacelleRGB[0])));
        }
        else {
          analogWrite(PIN_NACELLE_R, (255-(--oldNacelleRGB[0])));
        }
     }

     if ( oldNacelleRGB[1] != newNacelleRGB[1] ) {
        if ( oper_G ) {
          analogWrite(PIN_NACELLE_G, (255-(++oldNacelleRGB[1])));
        }
        else {
          analogWrite(PIN_NACELLE_G, (255-(--oldNacelleRGB[1])));
        }
     }

     if ( oldNacelleRGB[2] != newNacelleRGB[2] ) {
        if ( oper_B ) {
          analogWrite(PIN_NACELLE_B, (255-(++oldNacelleRGB[2])));
        }
        else {
          analogWrite(PIN_NACELLE_B, (255-(--oldNacelleRGB[2])));
        }
     }
}

void warp_engage() {

  long startTime = millis();
  long endTime = startTime + 2500;
  bool firstPass = true;
  int startPause = 0;
  int endPause = 0;
  
  for(byte tempBright=minBright; tempBright<maxBright; tempBright++) {
    long tempHue = colorSet[random((LED_COUNT-1))]; 
    for (int nPix = 0; nPix < LED_COUNT; nPix++){
      strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV((tempHue+(nPix+2500)), 255, tempBright)));
    } //end for
    
    strip.show(); // Update strip with new contents
    if (firstPass) {
      firstPass = false;
      startPause = (2750/(maxBright - minBright)) - (millis() - startTime); 
    }
    delay(startPause);  // pause for a moment
  }  

  startPause = true;
  
  for(byte decreaseBright=(maxBright+50); decreaseBright>(maxBright/2); decreaseBright--) {
    for (int npPix = 0; npPix < LED_COUNT; npPix++){
      bright[npPix]=decreaseBright;
      strip.setPixelColor(npPix, strip.gamma32(strip.ColorHSV(blue, 255, decreaseBright)));
    } //end for
    
    strip.show(); // Update strip with new contents
    if (firstPass) {
      firstPass = false;
      endPause = 500;
    }
    delay(endPause);
    endPause = 25;
  }
  
  resetHue(blue);
  bWarpOn=true;
}

void resetHue(long color) {
  for (byte hIndex = 0; hIndex < LED_COUNT; hIndex++) {
    hue[hIndex] = color; 
  }
}

void warp_cruise(){

    byte tempSat;
    byte tempBright;

    if (!bWarpOn) {
      return;
    }
    
    if ( (previousEngineMillis + 10 ) < millis() ) {
      previousEngineMillis = millis();
    }
    else {
      return;
    }
    
    for (int nPix = 0; nPix < LED_COUNT; nPix++){
      if (inout[nPix] == 1){
        tempBright = bright[nPix] + brightStep[nPix];
        if (tempBright >= maxBright) {
          inout[nPix] = 0;
          bright[nPix] = maxBright;
          brightStep[nPix] = random(2, 3);
        }
        else {
           bright[nPix] = tempBright;
        }
      }
      else {
        tempBright = bright[nPix] - brightStep[nPix];
        if (tempBright <= minBright) {
          inout[nPix] = 1;
          bright[nPix] = minBright;
          brightStep[nPix] = random(1, 3);
        }
        else {
          bright[nPix] = tempBright;
        }
      }       
      strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(hue[nPix], 255, bright[nPix])));
    } //end for
    
    strip.show(); // Update strip with new contents
}

void warp_decell(){

  bWarpOn = false;
  
  int tempBright = maxBright;

  long startColor = blue;
  long endColor = red;

  float colorDelta = (abs(endColor - startColor))/5;
  
  long startTime = millis();
  byte deadPos = 4; 
  byte deadPosCount = 0;
  int deadPosCountLimit = 1;
  
  while((millis() - startTime < 20000) && (tempBright > 10)) {

       for (byte pload = 0; pload < LED_COUNT; pload++){
         hue[pload] = startColor - ((LED_COUNT-1)-pload) * colorDelta;

         if (pload == deadPos) {
           bright[pload] = 0;
         }
         else {
           bright[pload] = tempBright; // - ((4-pload) * brightDelta);
         }
         
         strip.setPixelColor(pload, strip.gamma32(strip.ColorHSV(hue[pload], 255, bright[pload])));
       }

       if (deadPos > 0) {
          deadPos--;
       }
       else {
          deadPos = (LED_COUNT-1);
       }
       
       strip.show(); // Update strip with new contents
       
       ( tempBright > 0 ) ? tempBright-=1 : tempBright = 0;
       startColor += 100;
       endColor -=100; 
       colorDelta = (abs(endColor - startColor))/(LED_COUNT-1);
       delay(20);
  } //end while 
}

void runShutdownSequence(){
  bPowerOn = false;
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);  
  digitalWrite(PIN_BEACON_FLASHER, LOW);   
}

void runStartUpSequence() {
  //start nav lights
  bPowerOn = true;
}

void powerUpNacelle() {
  
}

void powerDownNacelle(){

  
}

void loop() {

   //if (!bWarpOn){
    //warp_engage();
   //}
   if (Serial.available() > 0) {
     // read the incoming byte:
     incomingByte = Serial.read();

     switch (incomingByte) {
       case SERIAL_COMM_POWER_OFF: 
          runShutdownSequence();
          break;
       case SERIAL_COMM_POWER_ON: //power on
          runStartUpSequence();
          break;
       case SERIAL_COMM_NAV_BEACON_ON:
          if (bPowerOn) {
            digitalWrite(PIN_BEACON_FLASHER, HIGH);
          }
          break;
       case SERIAL_COMM_NAV_BEACON_OFF:
          digitalWrite(PIN_BEACON_FLASHER, LOW);
          break;
       case SERIAL_COMM_NAV_FLASHER_ON:
          if (bPowerOn) {
            digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
          }
          break;
       case SERIAL_COMM_NAV_FLASHER_OFF:
          digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
          break;
       case SERIAL_COMM_NACELLE_COLOR:
          byte color[3];
          Serial.readBytes(&color[0],3);
          setNacelle(color);
          break;
       case SERIAL_COMM_INCREASE_WARP_DRIVE:
          warp_engage();
          break;
       case SERIAL_COMM_DECREASE_WARP_DRIVE:
          warp_decell();
          break;       
       case SERIAL_COMM_START_WARP_DRIVE:
          break;
       case SERIAL_COMM_STOP_WARP_DRIVE:
          warp_decell();
          powerDownNacelle();
          break;  
       case SERIAL_COMM_FLOOD_ENGINE_ON:
          digitalWrite(PIN_ENGINE_FLOOD, HIGH);
          break;   
       case SERIAL_COMM_FLOOD_ENGINE_OFF:
          digitalWrite(PIN_ENGINE_FLOOD, LOW);
          break;  
       case SERIAL_COMM_FLOOD_FORWARD_ON:
          digitalWrite(PIN_FORWARD_FLOOD, HIGH);
          break;  
       case SERIAL_COMM_FLOOD_FORWARD_OFF:
          digitalWrite(PIN_FORWARD_FLOOD, LOW);
          break;   
       default:
          break;
     }
   }

   warp_cruise();
}
