#include "Arduino.h"
#include <ButtonReader.h>
#include <PinChangeInt.h>
#include <EN1701-REFIT.h>

boolean lastState_power_switch_up = true;
boolean lastState_torpedo_switch_up = true;
boolean lastState_phaser_switch_up = true;
boolean lastState_deflector_switch_up = true;
boolean lastState_transporter_switch_up = true;
boolean lastState_red_alert_switch_up = true;
boolean lastState_speed_up_switch_up = true;
boolean lastState_speed_down_switch_up = true;

unsigned long last_power_DebounceTime = 0;
unsigned long last_torpedo_DebounceTime = 0;
unsigned long last_phaser_DebounceTime = 0;
unsigned long last_deflector_DebounceTime = 0;
unsigned long last_transporter_DebounceTime = 0;
unsigned long last_red_alert_DebounceTime = 0;
unsigned long last_speed_up_DebounceTime = 0;
unsigned long last_speed_down_DebounceTime = 0;

unsigned long buttonDebounceDelay = 50;

boolean b_warp_mode_on = false;
boolean b_red_alert_on = false;

#define PIN_TRANSPORTER 6
#define PIN_PHASER_BUTTON 7
#define PIN_TORPEDO_BUTTON 8
#define PIN_ALERT_MODE 9

#define PIN_POWER_CYCLE 14
#define PIN_DEFLECTOR_MODE 15
#define PIN_SPEED_UP 16
#define PIN_SPEED_DOWN 17
#define PIN_RED_ALERT 18
#define PIN_TEST_INTERRUPT 19

ButtonReader::ButtonReader() {
  pinMode(PIN_PHASER_BUTTON,INPUT_PULLUP);
  pinMode(PIN_TORPEDO_BUTTON,INPUT_PULLUP);
  pinMode(PIN_TEST_INTERRUPT, INPUT_PULLUP);
  pinMode(PIN_RED_ALERT,INPUT_PULLUP);
  pinMode(PIN_SPEED_DOWN,INPUT_PULLUP);
  pinMode(PIN_SPEED_UP,INPUT_PULLUP);
  pinMode(PIN_DEFLECTOR_MODE, INPUT_PULLUP);
  pinMode(PIN_TRANSPORTER,INPUT_PULLUP);
  pinMode(PIN_POWER_CYCLE, INPUT_PULLUP);
}

void ButtonReader::setupInterrupts() {
  PCintPort::attachInterrupt(PIN_TEST_INTERRUPT, testInterruptPin,CHANGE);
}

boolean ButtonReader::pollButtons(){
  /*boolean deflectorSwitch = digitalRead(PIN_DEFLECTOR_MODE);
  if (deflectorSwitch != lastState_deflector_switch_up){
      lastState_deflector_switch_up = deflectorSwitch;
      updateShipState_fromPolledButton(PIN_DEFLECTOR_MODE, deflectorSwitch);
      return true;
  }
  return false;*/
}

void ButtonReader::testInterruptPin() {
  EN1701A::buttonInterrupt = false;
  if (onButtonChange(PIN_POWER_CYCLE, lastState_power_switch_up, last_power_DebounceTime, true )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_TORPEDO_BUTTON, lastState_torpedo_switch_up, last_torpedo_DebounceTime, true )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_PHASER_BUTTON, lastState_phaser_switch_up, last_phaser_DebounceTime, false )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_DEFLECTOR_MODE, lastState_deflector_switch_up, last_deflector_DebounceTime, true )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_SPEED_DOWN, lastState_speed_down_switch_up, last_speed_down_DebounceTime, true )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_SPEED_UP, lastState_speed_up_switch_up, last_speed_up_DebounceTime, true )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_RED_ALERT, lastState_red_alert_switch_up, last_red_alert_DebounceTime, false )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChange(PIN_TRANSPORTER, lastState_transporter_switch_up, last_transporter_DebounceTime, true )) {
    EN1701A::buttonInterrupt = true;
    return;
  }
}

void ButtonReader::updateShipState_fromPolledButton(byte pinToTest, boolean newState)  {
   /*switch (pinToTest) {
     case PIN_DEFLECTOR_MODE:
       if (newState) {
         EN1701A::svWriteShipState(false, IMPULSE_ENGINES);
         EN1701A::svWriteShipState(true, WARP_ENGINES);
       }
       else {
         EN1701A::svWriteShipState(true, IMPULSE_ENGINES);
         EN1701A::svWriteShipState(false, WARP_ENGINES);
       }
       break;
   }*/
}

void ButtonReader::updateShipState_fromInterrupt(byte pinToTest, boolean newState)  {
   switch (pinToTest) {
     case PIN_POWER_CYCLE:
       EN1701A::svWriteShipState(newState, POWER_CHANGE);
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
     case PIN_DEFLECTOR_MODE:
       b_warp_mode_on = !b_warp_mode_on;
       if (b_warp_mode_on) {
         EN1701A::svWriteShipState(false, IMPULSE_ENGINES);
         EN1701A::svWriteShipState(true, WARP_ENGINES);
       }
       else {
         EN1701A::svWriteShipState(true, IMPULSE_ENGINES);
         EN1701A::svWriteShipState(false, WARP_ENGINES);
       }
       break;
     case PIN_SPEED_UP:
       if (b_warp_mode_on) {
         EN1701A::svWriteShipState(true, INCREASE_WARP_ENGINES);
       }
       /*EN1701A::svWriteShipState(newState, SR_PHASER);
       if (!newState){
         EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
       }*/
       break;
     case PIN_SPEED_DOWN:
       if (b_warp_mode_on) {
         EN1701A::svWriteShipState(true, DECREASE_WARP_ENGINES);
       }
       //EN1701A::svWriteShipState(newState, SR_TORPEDO);
       break;
     case PIN_RED_ALERT:
       b_red_alert_on = !b_red_alert_on;
       if ( b_red_alert_on ) {
         EN1701A::sbAudioIndex = AUDIO_INDEX_RED_ALERT;
         EN1701A::svWriteShipState(true, AUDIO_EFFECT);
       }
       else {
         EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
         EN1701A::svWriteShipState(false, AUDIO_EFFECT);
       }
       break;
     case PIN_TRANSPORTER:
       //EN1701A::svWriteShipState(newState, SR_TORPEDO);
       break;
   }
}

boolean ButtonReader::onButtonChange(byte pinToTest, boolean &stateTarget, unsigned long &debounceTarget, boolean bDownOnly ) {
  if ( digitalRead(pinToTest) == stateTarget ) {return false;}

  if (stateTarget) { //going down
    if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
      debounceTarget = millis();
      stateTarget = false;
      updateShipState_fromInterrupt(pinToTest, true);
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
        updateShipState_fromInterrupt(pinToTest,false);
        stateTarget = true;
        return true;
      }
    }
  }
  return false;
}
