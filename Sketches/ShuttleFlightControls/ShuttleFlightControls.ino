#include "Arduino.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define NEOPIXEL_PIN    18

#define COMMAND_PANEL_LED_1 13
#define COMMAND_PANEL_LED_2 12
#define COMMAND_PANEL_LED_3 11
#define COMMAND_PANEL_LED_4 10

#define NAV_PANEL_LED_1 9
#define NAV_PANEL_LED_2 7
#define NAV_PANEL_LED_3 5

#define SCANNER_HUD_LED 19
#define SCANNER_PANEL_LED 20

#define NAV_INDICATOR_LED_1 21
#define NAV_INDICATOR_LED_2 22
#define NAV_INDICATOR_LED_3 23

// How many NeoPixels are attached to the Arduino?
#define NEOPIXEL_COUNT 3

// Declare our NeoPixel strip object:
//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip(NEOPIXEL_COUNT, NEOPIXEL_PIN,NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

/*long colorSet[LED_COUNT] = {red, pink, blue, green};
long hue[LED_COUNT] = {blue, blue, blue, blue}; //0 (red) to 65535 (red)
byte sat[LED_COUNT] = {50, 255, 100, 230}; //(0-255) grey to full color 
byte bright[LED_COUNT] = {25, 40, 20, 60}; //(0-255) black to full bright
byte inout[LED_COUNT] = {1,0,1,0}; //0 or 1 
byte satStep[LED_COUNT] =  {1,1,1,1};
byte brightStep[LED_COUNT] = {1,2,1,2};
byte maxBright = 180;
byte minBright = 1;*/

void setup() {
  pinMode(COMMAND_PANEL_LED_1, OUTPUT);
  pinMode(COMMAND_PANEL_LED_2, OUTPUT);
  pinMode(COMMAND_PANEL_LED_3, OUTPUT);
  pinMode(COMMAND_PANEL_LED_4, OUTPUT);
  
  pinMode(NAV_PANEL_LED_1, OUTPUT);
  pinMode(NAV_PANEL_LED_2, OUTPUT);
  pinMode(NAV_PANEL_LED_3, OUTPUT);

  pinMode(SCANNER_HUD_LED, OUTPUT);
  pinMode(SCANNER_PANEL_LED, OUTPUT);

  pinMode(NAV_INDICATOR_LED_1, OUTPUT);
  pinMode(NAV_INDICATOR_LED_2, OUTPUT);
  pinMode(NAV_INDICATOR_LED_3, OUTPUT);

  digitalWrite(COMMAND_PANEL_LED_1, LOW);
  digitalWrite(COMMAND_PANEL_LED_2, LOW);
  digitalWrite(COMMAND_PANEL_LED_3, LOW);
  digitalWrite(COMMAND_PANEL_LED_4, LOW);

  digitalWrite(NAV_PANEL_LED_1, LOW);
  digitalWrite(NAV_PANEL_LED_2, LOW);
  digitalWrite(NAV_PANEL_LED_3, LOW);

  digitalWrite(SCANNER_HUD_LED, HIGH);
  digitalWrite(SCANNER_PANEL_LED, LOW);

  digitalWrite(NAV_INDICATOR_LED_1, LOW);
  digitalWrite(NAV_INDICATOR_LED_2, LOW);
  digitalWrite(NAV_INDICATOR_LED_3, LOW);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(60); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {

  console_pulse_panel_check();
  scanner_panel_check();
  nav_panel_check();
  nav_cluster_check();
  command_panel_check();
  
}

void command_panel_check(){
  static unsigned long current_command_time = 0;
  static unsigned long last_command_update_time = 0;
  static unsigned long command_delay_time = 50;
  static byte bitShifter = 0;

  static byte command_mode = 0;
  static unsigned long last_mode_update_time = 0;
  static unsigned long mode_delay_time = 3000;

  static bool shift_left = true;
  current_command_time = millis(); 

  if (current_command_time > last_mode_update_time + mode_delay_time ) {
    last_mode_update_time = current_command_time;
    switch(command_mode) {
      case 0:
        command_mode = 1;
        break;
      case 1:
        command_mode = 0;
        break;
      default: 
        break;
    }
  }

  switch(command_mode) {
     case 0:
       command_delay_time = 100;
       if (current_command_time > last_command_update_time + command_delay_time) {
          if (bitShifter == 0) {
            bitShifter=1;
          }
          else if ( bitRead(bitShifter, 3)){
            shift_left = false;
            bitShifter = bitShifter >> 1;
          }
          else if (bitRead(bitShifter, 0)){
            shift_left = true;
            bitShifter = bitShifter << 1;
          }
          else { 
            if (shift_left) {
              bitShifter = bitShifter << 1;
            }
            else {
              bitShifter = bitShifter >> 1;
            }
          }
          last_command_update_time = current_command_time;
          digitalWrite(COMMAND_PANEL_LED_1, bitRead(bitShifter,0));
          digitalWrite(COMMAND_PANEL_LED_2, bitRead(bitShifter,1));
          digitalWrite(COMMAND_PANEL_LED_3, bitRead(bitShifter,2));
          digitalWrite(COMMAND_PANEL_LED_4, bitRead(bitShifter,3));
        } 
        break;
      case 1:
        command_delay_time = 50;
        if (current_command_time > last_command_update_time + command_delay_time) {

           if (bitShifter == 0) {
            bitShifter= 1;
           }
           else if (bitRead(bitShifter, 3)){
             bitShifter = 1;
           }
           else { 
             bitShifter = bitShifter << 1;
           } 
           last_command_update_time = current_command_time;
           digitalWrite(COMMAND_PANEL_LED_1, bitRead(bitShifter,0));
           digitalWrite(COMMAND_PANEL_LED_2, bitRead(bitShifter,1));
           digitalWrite(COMMAND_PANEL_LED_3, bitRead(bitShifter,2));
           digitalWrite(COMMAND_PANEL_LED_4, bitRead(bitShifter,3));
         }
         break;
      case 3:
         break;  
      default:
         break;
  }
}

void nav_cluster_check(){
  static unsigned long current_cluster_time = 0;
  static unsigned long last_cluster_update_time = 0;
  static unsigned long cluster_delay_time = 350;

  long led_values = 7;

  current_cluster_time = millis(); 

  if (current_cluster_time > last_cluster_update_time + cluster_delay_time) {
     //nav_delay_time = 1000;
     led_values = random(0,7);
     last_cluster_update_time = current_cluster_time;
     digitalWrite(NAV_INDICATOR_LED_1, bitRead(led_values,0));
     digitalWrite(NAV_INDICATOR_LED_2, bitRead(led_values,1));
     digitalWrite(NAV_INDICATOR_LED_3, bitRead(led_values,2));
  }
}

void nav_panel_check(){
  static unsigned long current_nav_time = 0;
  static unsigned long last_nav_update_time = 0;
  static unsigned long nav_delay_time = 750;

  long led_values = 7;

  current_nav_time = millis(); 

  if (current_nav_time > last_nav_update_time + nav_delay_time) {
     //nav_delay_time = 1000;
     led_values = random(0,7);
     last_nav_update_time = current_nav_time;
     digitalWrite(NAV_PANEL_LED_1, bitRead(led_values,0));
     digitalWrite(NAV_PANEL_LED_2, bitRead(led_values,1));
     digitalWrite(NAV_PANEL_LED_3, bitRead(led_values,2));
  }
}

void scanner_panel_check(){
  static unsigned long current_scanner_time = 0;
  static unsigned long last_scanner_update_time = 0;
  static unsigned long scanner_delay_time = 3000;
  static bool scanner_on = false;

  current_scanner_time = millis(); 

  if (!scanner_on && 
      (current_scanner_time > last_scanner_update_time + scanner_delay_time)) {
     scanner_on = true;
     scanner_delay_time = 5000;
     last_scanner_update_time = current_scanner_time;
     digitalWrite(SCANNER_PANEL_LED, HIGH);
  }
  else if(scanner_on && 
          (current_scanner_time > last_scanner_update_time + scanner_delay_time)) {
     scanner_on = false;
     scanner_delay_time = 100;
     last_scanner_update_time = current_scanner_time;
     digitalWrite(SCANNER_PANEL_LED, LOW);
  }
}

void console_pulse_panel_check() {

  static byte R_values[3] = {127,0,110};
  static byte G_values[3] = {60,120,44};
  static byte B_values[3] = {20,0,127};

  static bool colorFlow_R[3] = {true, false, true};
  static bool colorFlow_G[3] = {false, true, true};
  static bool colorFlow_B[3] = {true, true, false};

  static uint32_t neoPixelColor[3] = { strip.Color(R_values[0], G_values[0], B_values[0]),
                       strip.Color(R_values[1], G_values[1], B_values[1]),
                       strip.Color(R_values[2], G_values[2], B_values[2]) };

  static unsigned long lastColorUpdateTime = 0;
  static unsigned long currentUpdateTime = millis();

  static bool oneShot = true;
  
  if (oneShot) {
    for(int n=0; n<strip.numPixels(); n++) { // For each pixel in strip...
        strip.setPixelColor(n, neoPixelColor[n]);
        strip.show();
    }
    oneShot = false;
  }


  if ((lastColorUpdateTime + 10) < currentUpdateTime) {

      for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
                          //  Update strip to match
        if (colorFlow_R[i]) {
          if (R_values[i] < 127) {
            R_values[i]++;
          }
          else {
            R_values[i]--;
            colorFlow_R[i] = false;
          }
        }
        else {
          if (R_values[i] > 0) {
            R_values[i]--;
          }
          else {
            R_values[i]++;
            colorFlow_R[i] = true;
          }
        }

        if (colorFlow_G[i]) {
          if (G_values[i] < 127) {
            G_values[i]++;
          }
          else {
            G_values[i]--;
            colorFlow_G[i] = false;
          }
        }
        else {
          if (G_values[i] > 0) {
            G_values[i]--;
          }
          else {
            G_values[i]++;
            colorFlow_G[i] = true;
          }
        }

        if (colorFlow_B[i]) {
          if (B_values[i] < 127) {
            B_values[i]++;
          }
          else {
            B_values[i]--;
            colorFlow_B[i] = false;
          }
        }
        else {
          if (B_values[i] > 0) {
            B_values[i]--;
          }
          else {
            B_values[i]++;
            colorFlow_B[i] = true;
          }
        }
        neoPixelColor[i] = strip.Color(R_values[i], G_values[i], B_values[i]);
        strip.setPixelColor(i, neoPixelColor[i]);         //  Set pixel's color (in RAM)
      } //end for
      
      lastColorUpdateTime = currentUpdateTime;
      strip.show();
   }// end if

   currentUpdateTime=millis();
 //}//end while
}
