#include "Arduino.h"
#include <ButtonReader_MK2.h>
#include <PinChangeInt.h>
#include <EN1701-REFIT_MK2.h>

unsigned long last_Button_A_DebounceTime = 0; //power
unsigned long last_Button_B_DebounceTime = 0; //speed up
unsigned long last_Button_C_DebounceTime = 0; //panel mode
unsigned long last_Button_D_DebounceTime = 0; //phasers

unsigned long last_Button_E_DebounceTime = 0; //deflector mode
unsigned long last_Button_F_DebounceTime = 0; //speed down
unsigned long last_Button_G_DebounceTime = 0; //red alert
unsigned long last_Button_H_DebounceTime = 0; //torpedos
unsigned long buttonDebounceDelay = 250;

ButtonReader_MK2::ButtonReader_MK2() {
  pinMode(PIN_TEST_INTERRUPT, INPUT_PULLUP);

  pinMode(PIN_A_BUTTON, INPUT_PULLUP);
  pinMode(PIN_B_BUTTON, INPUT_PULLUP);
  pinMode(PIN_C_BUTTON, INPUT_PULLUP);
  pinMode(PIN_D_BUTTON,INPUT_PULLUP);

  pinMode(PIN_E_BUTTON, INPUT_PULLUP);
  pinMode(PIN_F_BUTTON,INPUT_PULLUP);
  pinMode(PIN_G_BUTTON,INPUT_PULLUP);
  pinMode(PIN_H_BUTTON,INPUT_PULLUP);
}

void ButtonReader_MK2::setupInterrupts() {
  PCintPort::attachInterrupt(PIN_TEST_INTERRUPT, testInterruptPinFall, FALLING); //RISING, FALLING and CHANGE
}

void ButtonReader_MK2::testInterruptPinFall() {
  EN1701A::buttonInterrupt = false;
  if (onButtonChangeDown(PIN_A_BUTTON, last_Button_A_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_B_BUTTON, last_Button_B_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_C_BUTTON, last_Button_C_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_D_BUTTON, last_Button_D_DebounceTime )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_E_BUTTON, last_Button_E_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_F_BUTTON, last_Button_F_DebounceTime )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_G_BUTTON, last_Button_G_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_H_BUTTON, last_Button_H_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }
}

boolean ButtonReader_MK2::onButtonChangeDown(byte pinToTest, unsigned long &debounceTarget ) {
  if ( digitalRead(pinToTest) == false ) {

    if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
      debounceTarget = millis();
      //updateShipState_fromInterrupt(pinToTest);
      EN1701A::buttonPressed=pinToTest;
      return true;
    }
  }
  return false;
}
