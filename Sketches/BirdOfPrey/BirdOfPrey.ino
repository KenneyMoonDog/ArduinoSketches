#include "Arduino.h"
//#include <PinChangeInt.h>

volatile bool bPowerOn = false;

//timer constants
//#define RECEIVER_INTERRUPT_FREQ_FLASHER 100 //ms
//#define flashTimeOn 10
//#define flashTimeOff 11

#define PIN_ENG_1 3 //analog
#define PIN_ENG_2 5 //analog
#define PIN_NAV_LIGHTS 4 //analog
#define PIN_BUZZARDS 6 //analog
#define PIN_PLASMA_CANNON 9
#define PIN_WING_FLOOD_PORT 10 //analog
#define PIN_WING_FLOOD_STARBOARD 11 //analog
#define PIN_HANGER 12

#define BUTTON_PIN_1 14
#define BUTTON_PIN_2 15
#define BUTTON_PIN_3 16
//#define BUTTON_PIN_4 17
#define INTERRUPT_PIN 17



class timedOutputPinHandler {
    
  private:
  float no_radians = 0;
  
  public: 
  uint8_t PIN = 0;
     
  timedOutputPinHandler(uint8_t pin, float startRadians){
     PIN=pin;
     pinMode(PIN, OUTPUT); 
     digitalWrite(PIN, LOW);
     no_radians = startRadians;
  }

  uint8_t getPIN() {
    return PIN;
  }
  
  virtual void timedAction() {

    uint8_t pinValue=0;
    static int counter = 0;
    
    if (counter++ > 500){
      counter=0;
      if (no_radians < 3.13) {
        no_radians=no_radians+0.01;
      }
      else {
        no_radians=0;
      }

      Serial.print("Radians:");  
      Serial.println(no_radians);
      Serial.print("Sin:");  
      Serial.println(sin(no_radians));

      pinValue = round((1.0 + sin(no_radians)) * 125.0);
      analogWrite(PIN, pinValue);

      Serial.print("pinValue:");  
      Serial.println(pinValue);
    
    /*if ( rising ) {
      if (currentOutputValue < maxLimit  ){
        currentOutputValue++;
        analogWrite(PIN, currentOutputValue);
      }
      else {
        rising=!rising;
      }
    }
    else {
      if (currentOutputValue > minLimit  ){
        currentOutputValue--;
        analogWrite(PIN, currentOutputValue);
      }
      else {
        rising=!rising;
      }      
    }  */  
  
    }
  }
};

class WarpEngine : timedOutputPinHandler{
   private:
     bool rising = true;
      
   public:
     WarpEngine(uint8_t pin, float startRadian) : timedOutputPinHandler(pin, startRadian) {
     }

   void timedAction() {
      timedOutputPinHandler::timedAction();
   }
};

class plasmaCannon : timedOutputPinHandler{
   private:
     bool rising = true;
      
   public:
     plasmaCannon(uint8_t pin, float startRadian) : timedOutputPinHandler(pin, startRadian) {
     }

   void timedAction() {
      timedOutputPinHandler::timedAction();
   }
};

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

      return readButtonState();
    }

    virtual volatile uint8_t getPIN() {
      return PIN;
    }
}; //class jButton

plasmaCannon mCannon(PIN_PLASMA_CANNON,0);
WarpEngine mOuterEngine(PIN_ENG_1,0);
WarpEngine mInnerEngine(PIN_ENG_2,3.14);

jButton button_1(BUTTON_PIN_1);
jButton button_2(BUTTON_PIN_2);
jButton button_3(BUTTON_PIN_3);
//pinInterrupt button_trigger(INTERRUPT_PIN);

void setup() {    
  Serial.begin(9600);

  pinMode(PIN_NAV_LIGHTS, OUTPUT);   
  pinMode(PIN_BUZZARDS, OUTPUT); 
  pinMode(PIN_WING_FLOOD_PORT, OUTPUT); 
  pinMode(PIN_WING_FLOOD_STARBOARD, OUTPUT);
  pinMode(PIN_HANGER, OUTPUT);
  
  digitalWrite(PIN_NAV_LIGHTS, LOW);
  digitalWrite(PIN_BUZZARDS, LOW);
  digitalWrite(PIN_WING_FLOOD_PORT, LOW);
  digitalWrite(PIN_WING_FLOOD_STARBOARD, LOW);
  digitalWrite(PIN_HANGER, LOW);
  
  //interrupt time0 roughly halfway throuh 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  *digitalPinToPCMSK(INTERRUPT_PIN) |= bit (digitalPinToPCMSKbit(INTERRUPT_PIN));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(INTERRUPT_PIN)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(INTERRUPT_PIN)); // enable interrupt for the group

}//end setup

SIGNAL(TIMER0_COMPA_vect) //handler for timed interrupts
{
  mCannon.timedAction();
  mOuterEngine.timedAction();
  mInnerEngine.timedAction();
} 

ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{    
  // if I wired up D8-D13 then I'd need some code here
} 

ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here // NAV0
{
  if(button_1.onButtonChangeDown()){
  }

  if(button_2.onButtonChangeDown()){
  }

  if(button_3.onButtonChangeDown()){
  }
  // check it was NAV0 and nothing else
}

ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7 here // NAV1, NAV2
{
  // Check it was NAV1 or NAV2 and nothing else
}


/*void buttonPress() {
  button_1.onButtonChangeDown();
  button_2.onButtonChangeDown();
  button_3.onButtonChangeDown();
}*/

void startUp() {
 /* digitalWrite(PIN_BEACONS, HIGH);

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
  }   */
}

void shutDown() {
  /*digitalWrite(PIN_BEACONS, LOW);
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
  }   */

}

/*void updateNavBeacon(){

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
}*/



void loop() {
   
}
