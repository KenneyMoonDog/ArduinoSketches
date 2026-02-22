#include "Arduino.h"
#include <SparkFun_TB6612.h>
#include "PinChangeInterrupt.h"

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int motorNo = 1;
const int maxSpeed = 200;

#define CLOCKWISE 10
#define COUNTERCLOCKWISE 12
#define STOPPED 14
#define RUNNING 16
#define BRAKING 18
#define AUTO_OPERATION 20
#define CLOCKWISE_LIMITED 22
#define COUNTERCLOCKWISE_LIMITED 24

#define CONSOLE_POLLING_FREQUENCY 500 //ms
#define SPEED_INCREMENT 20

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

volatile unsigned long stateDebounceDelay = 100;
volatile unsigned long debounceTime = millis();

volatile int motorSpeed = 0;
volatile uint8_t motorDirection = CLOCKWISE;
volatile uint8_t motorState = STOPPED;
volatile bool reverseDirectionOnStop = false;

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
#define AIN1 3
#define AIN2 7
#define PWMA 5
#define STBY 9

#define BUTTON_TRN_CLKWISE 13
#define BUTTON_TRN_AUTO 12
#define BUTTON_TRN_CNTCLKWISE 11
#define LIMIT_SWITCH_LEFT 10
#define LIMIT_SWITCH_RIGHT 1

#define INTERRUPT_PIN 2

class jButton {
    
  private:
    int8_t PIN = 0;
    bool pressed = false;

  public:
    jButton(uint8_t pin) {
      PIN=pin;
      pinMode(PIN, INPUT_PULLUP);
    }

    bool readButtonState(){
       return pressed;
    }

    bool hasStateChanged() {

       bool currentButtonState = pressed;
       pressed = this->onButtonChange();

       if (currentButtonState != pressed){
         return true;
       }
       else {
         return false;
       }
    }

    bool onButtonChange(){
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

volatile jButton button_clk_wise(BUTTON_TRN_CLKWISE);
volatile jButton button_cnt_clk_wise(BUTTON_TRN_CNTCLKWISE);
volatile jButton button_auto_turn(BUTTON_TRN_AUTO);
volatile jButton limit_switch_left(LIMIT_SWITCH_LEFT);
volatile jButton limit_switch_right(LIMIT_SWITCH_RIGHT);

void setup() {
  Serial.begin(9600);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  //interrupt time0 roughly halfway throuh 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), buttonChangeHandler, CHANGE);
}

void buttonChangeHandler() {
  
  if ((millis() - debounceTime) > stateDebounceDelay) {

    Serial.println("EVENT");
    debounceTime = millis();

    if(limit_switch_left.hasStateChanged()){
      if (limit_switch_left.readButtonState()){
        Serial.println("Limit switch LEFT DOWN");
        motorState = COUNTERCLOCKWISE_LIMITED;
      }
      else {
        Serial.println("Limit switch LEFT UP");
      }
      return;
    }

    if(limit_switch_right.hasStateChanged()){
      if (limit_switch_right.readButtonState()){
        Serial.println("Limit switch RIGHT DOWN");
        motorState = CLOCKWISE_LIMITED;
      }
      else {
        Serial.println("Limit switch RIGHT UP");
      }
      return;
    }
  
    if(button_auto_turn.hasStateChanged()){
      if (button_auto_turn.readButtonState()){
        if (motorState == AUTO_OPERATION) {
          Serial.println("Toggling Auto OFF");
          reverseDirectionOnStop = false;
          motorState = BRAKING;
        }
        else {
          Serial.println("Toggling Auto ON");
          reverseDirectionOnStop = true;
          motorState = AUTO_OPERATION;
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
        motorState = BRAKING;
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
        motorState = BRAKING;
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
  currentMillis = millis();

  if (currentMillis - previousMillis >= CONSOLE_POLLING_FREQUENCY) {  //execute any timed operations every 250ms
    previousMillis = currentMillis; 

    if (motorState == AUTO_OPERATION) {
      if (motorSpeed < (maxSpeed - SPEED_INCREMENT)) {
        motorSpeed += SPEED_INCREMENT;

        if (motorDirection == CLOCKWISE){
          motor1.drive(motorSpeed);
        }
        else {
          motor1.drive(-(motorSpeed));
        }
      }
      return;
    }

    if (motorState == RUNNING) {
      reverseDirectionOnStop = false;
      if (motorSpeed < (maxSpeed - SPEED_INCREMENT)) {
        motorSpeed += SPEED_INCREMENT;

         if (motorDirection == CLOCKWISE){
          motor1.drive(motorSpeed);
         }
         else {
          motor1.drive(-(motorSpeed));
         }
      }
      return;
    }

    if (motorState == CLOCKWISE_LIMITED){
       motorState = BRAKING;
    }

    if (motorState == COUNTERCLOCKWISE_LIMITED){
       motorState = BRAKING;
    }

    if (motorState == BRAKING){
      //Serial.println("BRAKING");
      motorSpeed -= SPEED_INCREMENT;
      //Serial.println(motorSpeed);
      if (motorSpeed >= 0) {
        if (motorDirection == CLOCKWISE){
          motor1.drive(motorSpeed);
        }
        else {
          motor1.drive(-(motorSpeed));
        }

      } 
      else {
        //motor1.drive(0);
        Serial.println("STOPPED");
        motor1.brake();
        motor1.standby();
        motorState = STOPPED;  
        motorSpeed = 0;
      }

      if (motorState == STOPPED) {
        if (reverseDirectionOnStop) {
          if (motorDirection == CLOCKWISE){
            motorDirection = COUNTERCLOCKWISE;
          }
          else {
            motorDirection = CLOCKWISE;
          }
          motorState = AUTO_OPERATION;
        }
        Serial.println("reverse direction: ");
        Serial.println(reverseDirectionOnStop);
        Serial.println("DIRECTION: ");
        Serial.println(motorDirection);
        Serial.println("State:");
        Serial.println(motorState);
      } 
    }
  }

}

