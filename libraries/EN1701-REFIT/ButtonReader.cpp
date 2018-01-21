#include "Arduino.h"
#include <ButtonReader.h>
#include <PinChangeInt.h>
#include <EN1701-REFIT.h>

boolean powerState = false;
boolean lastState_power_Up = false;
boolean lastState_torpedo_Up = true;
boolean lastState_phaser_Up = true;

unsigned long last_power_DebounceTime = 0;
unsigned long last_torpedo_DebounceTime = 0;
unsigned long last_phaser_DebounceTime = 0;

unsigned long buttonDebounceDelay = 50;

#define PIN_DEFLECTOR_MODE 9
#define PIN_DEMO_MODE 8
#define PIN_TRANSPORTER 7
#define PIN_POWER_CYCLE 6

#define PIN_PHASER_BUTTON 14
#define PIN_TORPEDO_BUTTON 15
#define PIN_RED_ALERT 16
#define PIN_WARP_SPEED 17
#define PIN_IMPULSE_SPEED 18
#define PIN_TEST_INTERRUPT 19

ButtonReader::ButtonReader() {
  pinMode(PIN_PHASER_BUTTON,INPUT_PULLUP);
  pinMode(PIN_TORPEDO_BUTTON,INPUT_PULLUP);
  pinMode(PIN_TEST_INTERRUPT, INPUT_PULLUP);
  pinMode(PIN_RED_ALERT,INPUT_PULLUP);
  pinMode(PIN_WARP_SPEED,INPUT_PULLUP);
  pinMode(PIN_IMPULSE_SPEED, INPUT_PULLUP);
  pinMode(PIN_DEFLECTOR_MODE, INPUT_PULLUP);
  pinMode(PIN_DEMO_MODE,INPUT_PULLUP);
  pinMode(PIN_TRANSPORTER,INPUT_PULLUP);
  pinMode(PIN_POWER_CYCLE, INPUT_PULLUP);
}

void ButtonReader::setupInterrupts() {
  PCintPort::attachInterrupt(PIN_TEST_INTERRUPT, testInterruptPin,CHANGE);
}

void ButtonReader::testInterruptPin() {
  EN1701A::buttonInterrupt = false;
  if (onButtonChange(PIN_POWER_CYCLE, lastState_power_Up, last_power_DebounceTime, true )) { EN1701A::buttonInterrupt = true; return; }

  if (powerState) {
    if (onButtonChange(PIN_TORPEDO_BUTTON, lastState_torpedo_Up, last_torpedo_DebounceTime, true )) { EN1701A::buttonInterrupt = true; return; }
    if (onButtonChange(PIN_PHASER_BUTTON, lastState_phaser_Up, last_phaser_DebounceTime, false )) { EN1701A::buttonInterrupt = true; return; }
    //if (phaser_button_change()) { EN1701A::buttonInterrupt = true; return; }
  }
}

void ButtonReader::updateShipState(byte pinToTest, boolean newState)  {
   switch (pinToTest) {
     case PIN_POWER_CYCLE:
       EN1701A::svWriteShipState(newState, POWER_CHANGE);
       if (newState){
         powerState = true;
       }
       else {
         powerState = false;
       }
       break;
     case PIN_PHASER_BUTTON:
       EN1701A::svWriteShipState(newState, SR_PHASER);
       if (!newState){
         EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
       }
       break;
     case PIN_TORPEDO_BUTTON:
       EN1701A::svWriteShipState(newState, SR_TORPEDO);
       break;
   }
}

boolean ButtonReader::onButtonChange(byte pinToTest, boolean &stateTarget, unsigned long &debounceTarget, boolean bDownOnly ) {
  if ( digitalRead(pinToTest) == stateTarget ) {return false;}

  if (stateTarget) { //going down
    if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
      debounceTarget = millis();
      stateTarget = false;
      updateShipState(pinToTest, true);
      return true;
    }
  }
  else { //coming up
    if ( bDownOnly ) {
      if ((millis() - debounceTarget) > buttonDebounceDelay) {
        debounceTarget = millis();
        stateTarget = true;
      }
    }
    else {
      if ((millis() - debounceTarget) > buttonDebounceDelay) {
        debounceTarget = millis();
        updateShipState(pinToTest,false);
        stateTarget = true;
        return true;
      }
    }
  }
  return false;
}

boolean ButtonReader::torpedo_button_change() {
  if ( digitalRead(PIN_TORPEDO_BUTTON) == lastState_torpedo_Up ) {return false;}

  if (lastState_torpedo_Up) { //going down
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) { //then this should be a state change
      last_torpedo_DebounceTime = millis();
      lastState_torpedo_Up = false;
      EN1701A::svWriteShipState(true, SR_TORPEDO);
      return true;
      //EN1701A::mShipboardOperations.ApplyShipLogic();
      //pShipOperations->ApplyShipLogic();
    }
  }
  else { //coming up
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) {
      last_torpedo_DebounceTime = millis();
      lastState_torpedo_Up = true;
    }
  }
  return false;
}

/*boolean ButtonReader::torpedo_button_change() {
  if ( digitalRead(PIN_TORPEDO_BUTTON) == lastState_torpedo_Up ) {return false;}

  if (lastState_torpedo_Up) {
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) { //then this should be a state change
      last_torpedo_DebounceTime = millis();
      //EN1701A::svWriteShipState(true, SR_TORPEDO);
      lastState_torpedo_Up = false;
      return true;
    }
  }
  else {
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) {
      last_torpedo_DebounceTime = millis();
      lastState_torpedo_Up = true;
    }
  }
  return false;
}*/

boolean ButtonReader::phaser_button_change(){
  if ( digitalRead(PIN_PHASER_BUTTON) == lastState_phaser_Up ) {return false;}

  if (lastState_phaser_Up) {
    if ((millis() - last_phaser_DebounceTime) > buttonDebounceDelay) { //then this should be a state change
      last_phaser_DebounceTime = millis();
      EN1701A::svWriteShipState(true, SR_PHASER);
      lastState_phaser_Up = false;
      return true;
    }
  }
  else {
    if ((millis() - last_phaser_DebounceTime) > buttonDebounceDelay) {
      last_phaser_DebounceTime = millis();
      EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
      EN1701A::svWriteShipState(false, SR_PHASER);
      lastState_phaser_Up = true;
      return true;
    }
  }
  return false;
}
