#include <SERIAL_COMM_MK2.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

volatile unsigned long previousMillis = 0;
volatile unsigned long previousEngineMillis = 0;

int incomingByte = 0;   // for incoming serial data
byte newNacelleRGB[] = {10,10,10};
byte oldNacelleRGB[] = {0,0,0};

//byte newWarpPowerLevel = 25;
//byte oldWarpPowerLevel = 25;

int theaterChaseIncrement = 0;

boolean bWarpOn = false;
boolean enginePauseOn = true;
boolean bPowerOn = false;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN   9

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 5

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products

#define PIN_NAVIGATION_FLASHER 18
#define PIN_NACELLE_R 6
#define PIN_NACELLE_G 5
#define PIN_NACELLE_B 3

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 7 //ms

void setup() {    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_NACELLE_R, OUTPUT);
  pinMode(PIN_NACELLE_G, OUTPUT);
  pinMode(PIN_NACELLE_B, OUTPUT);
  
  analogWrite(PIN_NACELLE_R, 255);
  analogWrite(PIN_NACELLE_G, 255);
  analogWrite(PIN_NACELLE_B, 255);
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);

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
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {
    previousMillis = currentMillis;
    checkNacelleLevel();
  }

  if (currentMillis - previousEngineMillis >= (RECEIVER_INTERRUPT_FREQUENCY * 8)) {
    previousEngineMillis = currentMillis;

    theaterChase(strip.Color(0,0,127)); // Blue, half brightness
    if (theaterChaseIncrement++ == 3) {
      theaterChaseIncrement = 0;
    }  
  }
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

void runShutdownSequence(){
  bPowerOn = false;
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);   
}

void runStartUpSequence() {
  //start nav lights
  bPowerOn = true;
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color) {
    strip.clear();         //   Set all pixels in RAM to 0 (off)
    // 'c' counts up from 'b' to end of strip in steps of 3...
    for(int c=theaterChaseIncrement; c<strip.numPixels(); c += 3) {
      strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
    }
    strip.show(); // Update strip with new contents
}


void loop() {

   //test
   runStartUpSequence();
   //theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness
   
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
       case SERIAL_COMM_NAV_FLASHER_ON:
          if (bPowerOn) {
            digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
          }
          break;
       case SERIAL_COMM_NAV_FLASHER_OFF:
          digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
          break;
       case SERIAL_COMM_NACELLE_COLOR:
          Serial.readBytes(newNacelleRGB, 3);     
          break;
   /*    case SERIAL_COMM_INCREASE_WARP_DRIVE:
          coilPeriod+=25;
          resetWarpEngine();
          break;
       case SERIAL_COMM_DECREASE_WARP_DRIVE:
          coilPeriod-=25;
          resetWarpEngine();
          break;       
       case SERIAL_COMM_START_WARP_DRIVE:
          bWarpOn = true;
          break;
       case SERIAL_COMM_STOP_WARP_DRIVE:
          bWarpOn = false;
          break;*/     
       default:
          break;
     }
   }
}
