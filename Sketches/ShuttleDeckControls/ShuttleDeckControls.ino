#include "Arduino.h"
#include <SparkFun_TB6612.h>

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
#define AIN1 3
#define AIN2 7
#define PWMA 5
#define STBY 9

#define BUTTON_TRN_CLKWISE 18
#define BUTTON_TRN_AUTO 19
#define BUTTON_TRN_CNTCLKWISE 20
#define LIMIT_SWITCH_LEFT 21
#define LIMIT_SWITCH_RIGHT 22

#define INTERRUPT_PIN 2

class pinInterrupt {
  private:
    uint8_t PIN = 0;
    
  public:
    void *pHandler = 0;
  
    pinInterrupt(uint8_t pin) {
      PIN=pin;
      pinMode(PIN, INPUT_PULLUP);
      //pHandler = handler;

      //*digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(PIN));  // enable pin
      //PCIFR  |= bit (digitalPinToPCICRbit(PIN)); // clear any outstanding interrupt
      //PCICR  |= bit (digitalPinToPCICRbit(PIN)); // enable interrupt for the group
      //include PinChangeInt.h to support
      //intPort::attachInterrupt(PIN, pHandler, FALLING);
    }
};

class jButton {
    
  private:
    volatile uint8_t PIN = 0;
    volatile unsigned long buttonDebounceDelay = 250;
    volatile unsigned long debounceTarget = 0;
    volatile bool pressed = false;
    volatile bool resetOnRead = true;

  public:
    jButton(uint8_t pin) {
      PIN=pin;
      pinMode(PIN, INPUT_PULLUP);
    }

    virtual volatile bool readButtonState(){
      bool is_pressed = pressed;
      if (resetOnRead) {
        pressed = false;
      }

      return is_pressed;
    }

    virtual volatile void setResetOnRead(bool set) {
      resetOnRead = set;
    }

    virtual volatile bool onButtonChangeDown(){
      if ( digitalRead(PIN) == false ) {
        if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
          debounceTarget = millis();
          pressed = true;
        }
      }

      virtual volatile bool onButtonChangeUp(){
      if ( digitalRead(PIN) == true ) {
        if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
          debounceTarget = millis();
          pressed = false;
        }
      }

      return readButtonState();
    }

    virtual volatile uint8_t getPIN() {
      return PIN;
    }
}; //class jButton

// Initializing motors.  The library will allow you to initialize as many
// motors as you have memory for.  If you are using functions like forward
// that take 2 motors as arguements you can either write new functions or
// call the function more than once.
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
//Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

void setup() {
  // put your setup code here, to run once:

}

void loop()
{
   //Use of the drive function which takes as arguements the speed
   //and optional duration.  A negative speed will cause it to go
   //backwards.  Speed can be from -255 to 255.  Also use of the 
   //brake function which takes no arguements.
   motor1.drive(100,2000);
   motor1.brake();
   delay(2000);
   motor1.drive(-100,2000);
   motor1.brake();
   delay(2000);

   //Use of the drive function which takes as arguements the speed
   //and optional duration.  A negative speed will cause it to go
   //backwards.  Speed can be from -255 to 255.  Also use of the 
   //brake function which takes no arguements.
   //motor2.drive(255,1000);
   //motor2.drive(-255,1000);
   //motor2.brake();
   //delay(1000);

   //Use of the forward function, which takes as arguements two motors
   //and optionally a speed.  If a negative number is used for speed
   //it will go backwards
   //forward(motor1, motor2, 150);
   //delay(1000);

   //Use of the back function, which takes as arguments two motors 
   //and optionally a speed.  Either a positive number or a negative
   //number for speed will cause it to go backwards
   //back(motor1, motor2, -150);
   //delay(1000);

   //Use of the brake function which takes as arguments two motors.
   //Note that functions do not stop motors on their own.
   //brake(motor1, motor2);
   //elay(1000);

   //Use of the left and right functions which take as arguements two
   //motors and a speed.  This function turns both motors to move in 
   //the appropriate direction.  For turning a single motor use drive.
   //left(motor1, motor2, 100);
   //delay(1000);
   //right(motor1, motor2, 100);
   //delay(1000);

   //Use of brake again.
   //brake(motor1, motor2);
   //delay(1000);

}

