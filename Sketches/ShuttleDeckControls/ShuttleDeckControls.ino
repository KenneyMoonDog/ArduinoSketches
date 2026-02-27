#include "Arduino.h"
#include <SparkFun_TB6612.h>
#include "PinChangeInterrupt.h"
#include <Adafruit_NeoPixel.h>

const int motorNo = 1;
const int maxSpeed = 150;

//motor states and direction
#define CLOCKWISE 10
#define COUNTERCLOCKWISE 12
#define STOPPED 14
#define RUNNING 16
#define BRAKING 18
#define AUTO_OPERATION 20
#define CLOCKWISE_LIMITED 22
#define COUNTERCLOCKWISE_LIMITED 24
#define TIMED_REVERSE 26
#define SPEED_INCREMENT 10

#define CONSOLE_POLLING_FREQUENCY 100 //ms  how often we'll check for a change in state


unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

volatile unsigned long stateDebounceDelay = 50;
volatile unsigned long debounceTime = millis();

volatile int motorSpeed = 0;
volatile uint8_t motorDirection = CLOCKWISE;
volatile uint8_t motorState = STOPPED;
volatile bool reverseDirectionOnStop = false;
volatile bool counterclockwise_limit_tripped = false;
volatile bool clockwise_limit_tripped = false;

bool firstPass = true;

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
#define AIN1 18
#define AIN2 7
#define PWMA 5
#define STBY 9

//pins for buttons
#define BUTTON_TRN_CLKWISE 13
#define BUTTON_TRN_AUTO 12
#define BUTTON_TRN_CNTCLKWISE 11
#define LIMIT_SWITCH_LEFT 10
#define LIMIT_SWITCH_RIGHT 1

//we have two interrupts required for concurrent actions
#define INTERRUPT_PIN 2
#define INTERRUPT_LIMIT_PIN 3

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN   19
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
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

#define BUTX_CLOCKWISE 0
#define BUTX_COUNTERCLOCKWISE 2
#define BUTX_AUTO 1

#define MAXBRIGHT 255
#define THREEQUARTERBRIGHT 191
#define HALFBRIGHT 127
#define QUARTERBRIGHT 64
#define MINBRIGHT 20
#define OFF 0

class jButton {  //create an instance per button passing in the pin.  
    
  private:
    int8_t PIN = 0;
    bool pressed = false;
    byte indexNum=0;

  public:
    jButton(uint8_t pin, byte indx) {
      PIN=pin;
      pinMode(PIN, INPUT_PULLUP);
      indexNum=indx;
    }

    bool readButtonState(){
       return pressed;
    }

    byte getButtonIndex(){
      return indexNum;
    }

    bool hasStateChanged() {  //return true if the value of the button has changed

       bool currentButtonState = pressed;
       pressed = this->onButtonChange();

       if (currentButtonState != pressed){
         return true;
       }
       else {
         return false;
       }
    }

    bool onButtonChange(){  //read the digital pin for its value.  Note this assumes the signal is debounced properly
      delay(20);
      if ( digitalRead(PIN) == LOW ){
        return true;
      }
      else {
        return false;
      }
    }

    uint8_t getPIN() {
      return PIN;
    }
}; //class jButton

Motor motor1 = Motor(AIN1, AIN2, PWMA, motorNo, STBY);

//all buttons and/or switches
volatile jButton button_clk_wise(BUTTON_TRN_CLKWISE, BUTX_CLOCKWISE);
volatile jButton button_cnt_clk_wise(BUTTON_TRN_CNTCLKWISE, BUTX_COUNTERCLOCKWISE);
volatile jButton button_auto_turn(BUTTON_TRN_AUTO, BUTX_AUTO);
volatile jButton limit_switch_left(LIMIT_SWITCH_LEFT, BUTX_COUNTERCLOCKWISE);
volatile jButton limit_switch_right(LIMIT_SWITCH_RIGHT, BUTX_CLOCKWISE);

void setup() {
  Serial.begin(9600);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(INTERRUPT_LIMIT_PIN, INPUT_PULLUP);
  //interrupt time0 roughly halfway throuh 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), buttonChangeHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_LIMIT_PIN), limitChangeHandler, CHANGE);
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
}

void showStartupSequence() {
  
  for (byte passNo = 0; passNo < 3; passNo++){

  //do a pass from start to end of successive flashes
    for (int ledCount = 0; ledCount < LED_COUNT; ledCount++) {
      strip.setPixelColor(ledCount, (0, 0, HALFBRIGHT));
      if (ledCount > 0){
        strip.setPixelColor((ledCount-1), (0, 0, 0));
      }
      strip.show();
      delay(50);
    }

    //do a pass from end to start of successive flashes
    for (int ledCount = (LED_COUNT-1); ledCount >= 0; ledCount--) {
      strip.setPixelColor(ledCount,(0, 0, HALFBRIGHT));
      if (ledCount < LED_COUNT){
        strip.setPixelColor((ledCount + 1), (0, 0, 0));
      }
      strip.show();
      delay(50);
    }
    delay(50);
    setAndShowPixel(0,0,0,0);
  }

  for (int newBright = 0; newBright <= MAXBRIGHT; newBright++){
    for (int ledCount = 0; ledCount < LED_COUNT; ledCount++) {
      strip.setPixelColor(ledCount, (0, 0, newBright));
    }
    strip.show();
    delay(2);
  }

  for (int newBright = MAXBRIGHT; newBright >= QUARTERBRIGHT; newBright--){
    for (int ledCount = 0; ledCount < LED_COUNT; ledCount++) {
      strip.setPixelColor(ledCount, (0, 0, newBright));
    }
    strip.show();
    delay(10);
  }
}

void setAndShowPixel(byte pixelNumber, int R, int G, int B){
  strip.setPixelColor(pixelNumber, (R,G,B));
  strip.show();
}

//void setAndShowPixel(byte pixelNumber, long color, byte sat, byte brightness){
//    strip.setPixelColor(pixelNumber, strip.gamma32(strip.ColorHSV(color, sat, brightness)));
//    strip.show(); // Update strip with new contents
//}

void resetAllButtonLights(){
    for (int ledCount = 0; ledCount < LED_COUNT; ledCount++) {
      strip.setPixelColor(ledCount, (0,0,QUARTERBRIGHT));
    }
    strip.show();
}

//this handler responds only to changes in the rotation limited switches
void limitChangeHandler() {

  //once detected, act on the first occurance only and ignore any spurious signals for 'stateDebounceDelay' ms
  if ((millis() - debounceTime) > stateDebounceDelay) {
    debounceTime = millis();

    if(limit_switch_left.hasStateChanged()){
      if (limit_switch_left.readButtonState()){
        Serial.println("Counterclockwise Limit switch DOWN");
        motorState = COUNTERCLOCKWISE_LIMITED;
      }
      else {
        Serial.println("Counterclockwise Limit switch UP");
        counterclockwise_limit_tripped = false;
      }
      return;
    }

    if(limit_switch_right.hasStateChanged()){
      if (limit_switch_right.readButtonState()){
        Serial.println("Clockwise Limit switch switch DOWN");
        motorState = CLOCKWISE_LIMITED;
      }
      else {
        Serial.println("Clockwise Limit switch switch UP");
        clockwise_limit_tripped = false;
      }
      return;
    }
  }
}

void buttonChangeHandler() {
  
  //once detected, act on the first occurance only and ignore any spurious signals for 'stateDebounceDelay' ms
  if ((millis() - debounceTime) > stateDebounceDelay) {

    debounceTime = millis();
  
    if(button_auto_turn.hasStateChanged()){
      if (button_auto_turn.readButtonState()){
        if (motorState == AUTO_OPERATION) {
          Serial.println("Toggling Auto OFF");
          reverseDirectionOnStop = false;
          motorState = BRAKING;

        }
        else {
          if (motorState == RUNNING) {  //if auto buttton invoked while the motor is already running, everything stops
            Serial.println("AUTO button interrupted RUN status");
            motorState = BRAKING;
          }
          else {
            Serial.println("Toggling Auto ON");
            reverseDirectionOnStop = true;
            motorState = AUTO_OPERATION;
          }
        }
      }
      else {
        Serial.println("AUTO UP");
      }
      return;
    }

    if(button_clk_wise.hasStateChanged()){
      if (button_clk_wise.readButtonState()){
        Serial.println("CLOCKWISE DOWN");
        reverseDirectionOnStop = false;
        motorDirection = CLOCKWISE;
        motorState = RUNNING;
      }
      else {
        Serial.println("CLOCKWISE UP");
        if( motorState != CLOCKWISE_LIMITED || motorState != CLOCKWISE_LIMITED) {
          motorState = BRAKING;
        }
      }
      return;
   }

    if(button_cnt_clk_wise.hasStateChanged()){
      if (button_cnt_clk_wise.readButtonState()){
        Serial.println("COUNTER CLOCKWISE DOWN");
        reverseDirectionOnStop = false;
        motorDirection = COUNTERCLOCKWISE;
        motorState = RUNNING;
      }
      else {
        Serial.println("COUNTER CLOCKWISE UP");
        if( motorState != CLOCKWISE_LIMITED || motorState != CLOCKWISE_LIMITED) {
          motorState = BRAKING;
        }
      }
      return;
    }
  } 
}

SIGNAL(TIMER0_COMPA_vect) //handler for timed interrupts
{
} 

void loop()
{

  if (firstPass){
      showStartupSequence();
      firstPass = false;
  }

  currentMillis = millis();

  if (currentMillis - previousMillis >= CONSOLE_POLLING_FREQUENCY) {  //execute any timed operations every X ms
    previousMillis = currentMillis; 

    if (motorState == TIMED_REVERSE) { 
      Serial.print("TIMED REVERSE");
      reverseDirectionOnStop = false;

      if (motorSpeed < (maxSpeed - SPEED_INCREMENT)) { //reverse
        motorSpeed += SPEED_INCREMENT;

        if (motorDirection == CLOCKWISE){
          motor1.drive(motorSpeed);
          strip.setPixelColor(button_cnt_clk_wise.getButtonIndex(),MAXBRIGHT,0,0);
          strip.show();
        }
        else {
          motor1.drive(-(motorSpeed));
          strip.setPixelColor(button_clk_wise.getButtonIndex(),MAXBRIGHT,0,0);
          strip.show();
        }
      }
      else { 
        delay(3000);
        motorState = BRAKING;
      }
    }

    if (motorState == AUTO_OPERATION) {
      strip.setPixelColor(button_auto_turn.getButtonIndex(),0,255,0);
      strip.show();

      if (motorSpeed < (maxSpeed - SPEED_INCREMENT)) {
        motorSpeed += SPEED_INCREMENT;

        if (motorDirection == CLOCKWISE){
          strip.setPixelColor(button_cnt_clk_wise.getButtonIndex(),0,0,QUARTERBRIGHT);
          strip.setPixelColor(button_clk_wise.getButtonIndex(),255,165,0); //orange
          strip.show();
          motor1.drive(motorSpeed);
        }
        else {
          motor1.drive(-(motorSpeed));
          strip.setPixelColor(button_cnt_clk_wise.getButtonIndex(),255,165,0); //orange
          strip.setPixelColor(button_clk_wise.getButtonIndex(),0,0,QUARTERBRIGHT);
          strip.show();
        }
      }
      return;
    }

    if (motorState == RUNNING) {
      reverseDirectionOnStop = false;
      if (motorSpeed < (maxSpeed - SPEED_INCREMENT)) {
        motorSpeed += SPEED_INCREMENT;

        if (motorDirection == CLOCKWISE){
          strip.setPixelColor(button_clk_wise.getButtonIndex(),0,MAXBRIGHT,0);
          strip.show();
          motor1.drive(motorSpeed);
        }
        else {
          strip.setPixelColor(button_cnt_clk_wise.getButtonIndex(),0,MAXBRIGHT,0);
          strip.show();
          motor1.drive(-(motorSpeed));
        }
      }
      return;
    }

    if (motorState == CLOCKWISE_LIMITED){
      clockwise_limit_tripped =  true;
      motorState = BRAKING;
    }

    if (motorState == COUNTERCLOCKWISE_LIMITED){
      counterclockwise_limit_tripped =  true;
      motorState = BRAKING;
    }

    if (motorState == BRAKING){
      motorSpeed -= SPEED_INCREMENT;
      if (motorSpeed >= 0) {
        if (motorDirection == CLOCKWISE){
          motor1.drive(motorSpeed);
        }
        else {
          motor1.drive(-(motorSpeed));
        }

      } 
      else {
        Serial.println("STOPPED");
        motor1.brake();
        motor1.standby();
        motorState = STOPPED;  
        motorSpeed = 0;
        resetAllButtonLights();
      }

      if (motorState == STOPPED) {
        resetAllButtonLights();
        
        if (reverseDirectionOnStop) {  //this should only be true if the former motorState was AUTO
          if (motorDirection == CLOCKWISE){
            motorDirection = COUNTERCLOCKWISE;
            strip.setPixelColor(button_cnt_clk_wise.getButtonIndex(),255,165,0); //orange
            strip.setPixelColor(button_clk_wise.getButtonIndex(),0,0,QUARTERBRIGHT);
            strip.show();
          }
          else {
            motorDirection = CLOCKWISE;
            strip.setPixelColor(button_clk_wise.getButtonIndex(),255,165,0); //orange
            strip.setPixelColor(button_clk_wise.getButtonIndex(),0,0,QUARTERBRIGHT);
            strip.show();
          }
          motorState = AUTO_OPERATION;

          Serial.println("Limited reached. Reverse direction: ");
          Serial.println(reverseDirectionOnStop);
          Serial.println("DIRECTION: ");
          Serial.println(motorDirection);
          Serial.println("State:");
          Serial.println(motorState);
        }
        else {  //then the rotation MAY have been stopped due to a limit swtch, back it out
          if (clockwise_limit_tripped == true) {
            strip.setPixelColor(button_clk_wise.getButtonIndex(),MAXBRIGHT,0,0);
            strip.show();
            Serial.println("CLOCKWISE Limited reached. Back up 3 seconds ");
            clockwise_limit_tripped = false;
            motorDirection = COUNTERCLOCKWISE;
            motorState = TIMED_REVERSE;
          }

          if (counterclockwise_limit_tripped == true) {
            strip.setPixelColor(button_cnt_clk_wise.getButtonIndex(),MAXBRIGHT,0,0);
            strip.show();
            Serial.println("COUNTERCLOCKWISE Limited reached. Back up 3 seconds ");
            counterclockwise_limit_tripped = false;
            motorDirection = CLOCKWISE; 
            motorState = TIMED_REVERSE;
          }
        }
      } 
    }
  }

}

