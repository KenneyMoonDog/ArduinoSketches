#include "Arduino.h"
#include <ButtonReader.h>
#include <PinChangeInt.h>
#include <EN1701-REFIT.h>
#include "ShipOperations.h"

volatile boolean lastState_torpedo_Up = true;
volatile boolean lastState_phaser_Up = true;

unsigned long last_torpedo_DebounceTime = 0;
unsigned long last_phaser_DebounceTime = 0;

unsigned long buttonDebounceDelay = 50;

#define PIN_PHASER_BUTTON 14
#define PIN_TORPEDO_BUTTON 15
#define PIN_INTERRUPT 19

ShipOperations EN1701A::mShipboardOperations;

ButtonReader::ButtonReader() {
  pinMode(PIN_PHASER_BUTTON,INPUT_PULLUP);
  pinMode(PIN_TORPEDO_BUTTON,INPUT_PULLUP);
  pinMode(PIN_INTERRUPT, INPUT_PULLUP);
}

void ButtonReader::setupInterrupts() {
  PCintPort::attachInterrupt(PIN_INTERRUPT, torpedo_button_interrupt, CHANGE);
  //PCintPort::attachInterrupt(PIN_TORPEDO_BUTTON, torpedo_button_change,CHANGE);
}

void ButtonReader::interruptTest(){
  //torpedo_button_change();
}

boolean ButtonReader::scanButtons() {
   boolean returnVal = false;
  // if (torpedo_button_change()) { returnVal = true; }
   if (phaser_button_change()) { returnVal = true; }

   return returnVal;
}

void ButtonReader::torpedo_button_interrupt() {
  if (lastState_torpedo_Up) {
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) { //then this should be a state change
      last_torpedo_DebounceTime = millis();
      lastState_torpedo_Up = false;
      EN1701A::svWriteShipState(true, SR_TORPEDO);
      EN1701A::mShipboardOperations.ApplyShipLogic();
      //pShipOperations->ApplyShipLogic();
    }
  }
  else {
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) {
      last_torpedo_DebounceTime = millis();
      lastState_torpedo_Up = true;
    }
  }
}

boolean ButtonReader::torpedo_button_change() {
  if ( digitalRead(PIN_TORPEDO_BUTTON) == lastState_torpedo_Up ) {return false;}

  if (lastState_torpedo_Up) {
    if ((millis() - last_torpedo_DebounceTime) > buttonDebounceDelay) { //then this should be a state change
      last_torpedo_DebounceTime = millis();
      EN1701A::svWriteShipState(true, SR_TORPEDO);
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
}

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
