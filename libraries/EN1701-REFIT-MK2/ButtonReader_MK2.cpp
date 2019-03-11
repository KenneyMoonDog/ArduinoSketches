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
unsigned long buttonDebounceDelay = 100;

#define MODE_HELM 1
#define MODE_COMMUNICATIONS 2
#define MODE_TRANSPORTER 3

#define PIN_A_BUTTON 7
#define PIN_B_BUTTON 14
#define PIN_C_BUTTON 17
#define PIN_D_BUTTON 15

#define PIN_E_BUTTON 8
#define PIN_F_BUTTON 6
#define PIN_G_BUTTON 18
#define PIN_H_BUTTON 16

#define PIN_TEST_INTERRUPT 19

ButtonReader_MK2::ButtonReader_MK2() {
  setMode(MODE_HELM);
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

void ButtonReader_MK2::setMode(byte mode){
  EN1701A::b_console_mode = mode;
  switch (mode) {
    case MODE_HELM:
      break;
    case MODE_TRANSPORTER:
      break;
    case MODE_COMMUNICATIONS:
      break;
    default:
      break;
  }
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

void ButtonReader_MK2::updateShipState_fromInterrupt(byte pinToTest)  {
   switch (pinToTest) {
     case PIN_A_BUTTON:
       EN1701A::b_power_cycle = !EN1701A::b_power_cycle;
       EN1701A::svWriteShipState(EN1701A::b_power_cycle, POWER_CHANGE);
       break;
     case PIN_B_BUTTON:
       if (EN1701A::b_warp_mode_on) {
         EN1701A::svWriteShipState(true, INCREASE_WARP_ENGINES);
       }
       else {
         EN1701A::svWriteShipState(true, INCREASE_IMPULSE_ENGINES);
       }
       break;
     case PIN_C_BUTTON:
       //EN1701A::svWriteShipState(buttonDown, SR_TORPEDO);
       break;
     case PIN_D_BUTTON:
       EN1701A::b_phaser_on = !EN1701A::b_phaser_on;
       if (EN1701A::b_phaser_on) {
         EN1701A::svWriteShipState(true, PHASER_ON);
         EN1701A::svWriteShipState(false, PHASER_OFF);
       }
       else {
         EN1701A::svWriteShipState(true, PHASER_OFF);
         EN1701A::svWriteShipState(false, PHASER_ON);
       }
       break;
     case PIN_E_BUTTON:
       EN1701A::b_warp_mode_on = !EN1701A::b_warp_mode_on;
       if (EN1701A::b_warp_mode_on) {
         //EN1701A::svWriteShipState(false, IMPULSE_ENGINES);
         //EN1701A::svWriteShipState(true, WARP_ENGINES);
         EN1701A::svWriteShipState(true, SWITCH_TO_WARP_MODE);
         EN1701A::svWriteShipState(false, SWITCH_TO_IMPULSE_MODE);
       }
       else {
         //EN1701A::svWriteShipState(true, IMPULSE_ENGINES);
         //EN1701A::svWriteShipState(false, WARP_ENGINES);
         EN1701A::svWriteShipState(true, SWITCH_TO_IMPULSE_MODE);
         EN1701A::svWriteShipState(false, SWITCH_TO_WARP_MODE);
       }
       break;
     case PIN_F_BUTTON:
       if (EN1701A::b_warp_mode_on) {
         EN1701A::svWriteShipState(true, DECREASE_WARP_ENGINES);
       }
       else {
         EN1701A::svWriteShipState(true, DECREASE_IMPULSE_ENGINES);
       }
       break;
     case PIN_G_BUTTON:
        EN1701A::b_red_alert_on = !EN1701A::b_red_alert_on;
        if ( EN1701A::b_red_alert_on ) {
           EN1701A::sbAudioIndex = AUDIO_INDEX_RED_ALERT;
           EN1701A::svWriteShipState(true, AUDIO_EFFECT);
        }
        else {
         EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
         EN1701A::svWriteShipState(false, AUDIO_EFFECT);
        }
        break;
     case PIN_H_BUTTON:
       EN1701A::svWriteShipState(true, TORPEDO);
       break;
   }
}

boolean ButtonReader_MK2::onButtonChangeDown(byte pinToTest, unsigned long &debounceTarget ) {
  if ( digitalRead(pinToTest) == false ) {

    if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
      debounceTarget = millis();
      updateShipState_fromInterrupt(pinToTest);
      return true;
    }
  }
  return false;
}
