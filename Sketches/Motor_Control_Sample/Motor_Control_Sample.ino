#include "Arduino.h"
#include <SparkFun_TB6612.h>
#include "PinChangeInterrupt.h"

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

volatile unsigned long stateDebounceDelay = 100;
volatile unsigned long debounceTime = millis();

volatile int motorSpeed = 0;
volatile uint8_t motorDirection = CLOCKWISE;
volatile uint8_t motorState = STOPPED;
volatile bool reverseDirectionOnStop = false;
volatile bool counterclockwise_limit_tripped = false;
volatile bool clockwise_limit_tripped = false;

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

class jButton {  //create an instance per button passing in the pin.  
    
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
volatile jButton button_clk_wise(BUTTON_TRN_CLKWISE);
volatile jButton button_cnt_clk_wise(BUTTON_TRN_CNTCLKWISE);
volatile jButton button_auto_turn(BUTTON_TRN_AUTO);
volatile jButton limit_switch_left(LIMIT_SWITCH_LEFT);
volatile jButton limit_switch_right(LIMIT_SWITCH_RIGHT);

void setup() {
  Serial.begin(9600);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(INTERRUPT_LIMIT_PIN, INPUT_PULLUP);
  //interrupt time0 roughly halfway throuh 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), buttonChangeHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_LIMIT_PIN), limitChangeHandler, CHANGE);
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

  if (currentMillis - previousMillis >= CONSOLE_POLLING_FREQUENCY) {  //execute any timed operations every X ms
    previousMillis = currentMillis; 

    if (motorState == TIMED_REVERSE) { 
      Serial.print("TIMED REVERSE");
      reverseDirectionOnStop = false;

      if (motorSpeed < (maxSpeed - SPEED_INCREMENT)) { //reverse
        motorSpeed += SPEED_INCREMENT;

        if (motorDirection == CLOCKWISE){
          motor1.drive(motorSpeed);
        }
        else {
          motor1.drive(-(motorSpeed));
        }
      }
      else { 
        delay(3000);
        motorState = BRAKING;
      }
    }

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
      }

      if (motorState == STOPPED) {
        if (reverseDirectionOnStop) {  //this should only be true if the former motorState was AUTO
          if (motorDirection == CLOCKWISE){
            motorDirection = COUNTERCLOCKWISE;
          }
          else {
            motorDirection = CLOCKWISE;
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
            Serial.println("CLOCKWISE Limited reached. Back up 3 seconds ");
            clockwise_limit_tripped = false;
            motorDirection = COUNTERCLOCKWISE;
            motorState = TIMED_REVERSE;
          }

          if (counterclockwise_limit_tripped == true) {
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

