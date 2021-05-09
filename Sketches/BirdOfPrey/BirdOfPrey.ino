#include "Arduino.h"
#include <PinChangeInt.h>

volatile unsigned long flasherPreviousMillis = 0;
volatile bool bPowerOn = false;

uint8_t currentBright = 0;

//timer constants
#define RECEIVER_INTERRUPT_FREQ_FLASHER 100 //ms
#define flashTimeOn 10
#define flashTimeOff 11

#define PIN_FLOODS 6 //analog
#define PIN_NACELLES 9 //analog
#define PIN_ENGINE 10 //analog
#define PIN_PLASMA_CANNON 11 //analog
#define PIN_BEACONS 12
#define PIN_NAVIGATION_FLASHER 13

#define BUTTON_PIN_1 5
#define BUTTON_PIN_2 4
#define BUTTON_PIN_INTERRUPT 3

class jButton {
  public:
    jButton(uint8_t pin) {
      PIN=pin;
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

    virtual volatile void onButtonChangeDown(){
      if ( digitalRead(PIN) == false ) {
        if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
          debounceTarget = millis();
          //Serial.println("BUTTON PRESS TRUE");
          pressed = true;
        }
      }
    }

    virtual volatile uint8_t getPIN() {
      return PIN;
    }
  
  private:
    volatile uint8_t PIN = 0;
    volatile unsigned long buttonDebounceDelay = 250;
    volatile unsigned long debounceTarget = 0;
    volatile bool pressed = false;
    volatile bool resetOnRead = true;
}; //class jButton

jButton button_1(BUTTON_PIN_1);
jButton button_2(BUTTON_PIN_2);

void buttonPress() {
  button_1.onButtonChangeDown();
  button_2.onButtonChangeDown();
}

void setup() {    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_BEACONS, OUTPUT);
  pinMode(PIN_ENGINE, OUTPUT);   
  pinMode(PIN_FLOODS, OUTPUT); 
  pinMode(PIN_PLASMA_CANNON, OUTPUT);
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
  digitalWrite(PIN_BEACONS, LOW);
  digitalWrite(PIN_ENGINE, LOW);
  digitalWrite(PIN_FLOODS, LOW);
  digitalWrite(PIN_PLASMA_CANNON, LOW);

  pinMode(BUTTON_PIN_INTERRUPT, INPUT_PULLUP);
  //enablePinInterupt(BUTTON_PIN_INTERRUPT);
  PCintPort::attachInterrupt(BUTTON_PIN_INTERRUPT, buttonPress, FALLING);

  pinMode(button_1.getPIN(), INPUT_PULLUP);
  pinMode(button_2.getPIN(), INPUT_PULLUP);
  
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

  enablePlasmaCannon(5, 50);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - flasherPreviousMillis >= RECEIVER_INTERRUPT_FREQ_FLASHER) {  //execute any timed operations every INTERRRUPT FREQ ms
    // save the last time you did a repeatable item clear
    flasherPreviousMillis = currentMillis;
    updateNavBeacon();
  } //end if timer
} 

void enablePinInterupt(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void startUp() {
  digitalWrite(PIN_BEACONS, HIGH);

  for (int brightness=15; brightness<=255; brightness+=1){
      analogWrite(PIN_NACELLES, brightness);
      delay(5);
  }   
  
  for (int brightness=0; brightness<=255; brightness+=1){
      analogWrite(PIN_ENGINE, brightness);
      delay(5);
  }   

  for (int brightness=0; brightness<=200; brightness+=1){
      analogWrite(PIN_FLOODS, brightness);
      delay(5);
  }   
}

void shutDown() {
  digitalWrite(PIN_BEACONS, LOW);
  delay(1000);

  for (int brightness=200; brightness>=0; brightness-=1){
      analogWrite(PIN_FLOODS, brightness);
      delay(10);
  }   
  
  for (int brightness=255; brightness>=0; brightness-=1){
      analogWrite(PIN_ENGINE, brightness);
      delay(10);
  }   

  for (int brightness=255; brightness>=5; brightness-=1){
      analogWrite(PIN_NACELLES, brightness);
      delay(10);
  }   

}

void updateNavBeacon(){

  static volatile byte bCounter=0;
  static volatile boolean bFlasherOn=false;

  if (bPowerOn){
    if (bCounter >= flashTimeOn && bCounter < flashTimeOff ){
       if (  !bFlasherOn ){
         digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
         bFlasherOn = true;
       }
    }

    if ( bFlasherOn && bCounter >= flashTimeOff ) {
       digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
       bCounter = -1;
       bFlasherOn = false;  
    }  
    bCounter++; 
  }
}

void enablePlasmaCannon( uint8_t maxBright, uint8_t loopDelay ){
   for (currentBright; currentBright<=maxBright; currentBright+=1){
      analogWrite(PIN_PLASMA_CANNON, currentBright);
      delay(loopDelay);
   }   
}

void disablePlasmaCannon (uint8_t minBright, uint8_t loopDelay){
   for (currentBright; currentBright>=minBright; currentBright-=1){
      analogWrite(PIN_PLASMA_CANNON, currentBright);
      delay(loopDelay);
   }  
}

void firePlasmaCannon(){
   enablePlasmaCannon(30, 50);
   digitalWrite(PIN_PLASMA_CANNON, HIGH);
   delay(100);
   disablePlasmaCannon(1,25);
}

void loop() {
   if (button_1.readButtonState()) { //Power Button

     if (bPowerOn == false) {
       bPowerOn = true;
       startUp();
     } 
     else {
       bPowerOn = false;
       shutDown();
     }
   }
   else if (button_2.readButtonState()) { //fire plasma 
     //Serial.println("BUTTON2");
     firePlasmaCannon();
   }
}
