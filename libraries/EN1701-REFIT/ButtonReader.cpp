#include "Arduino.h"
#include <ButtonReader.h>
#include <PinChangeInt.h>
#include <EN1701-REFIT.h>

unsigned long last_power_DebounceTime = 0;
unsigned long last_torpedo_DebounceTime = 0;
unsigned long last_phaser_DebounceTime = 0;
unsigned long last_deflector_DebounceTime = 0;
unsigned long last_panelmode_DebounceTime = 0;
unsigned long last_red_alert_DebounceTime = 0;
unsigned long last_speed_up_DebounceTime = 0;
unsigned long last_speed_down_DebounceTime = 0;

unsigned long buttonDebounceDelay = 100;

#define PIN_POWER_CYCLE 7
#define PIN_DEFLECTOR_MODE 8
#define PIN_SPEED_UP 14
#define PIN_SPEED_DOWN 6
#define PIN_RED_ALERT 18
#define PIN_PANEL_MODE 17
#define PIN_PHASER_BUTTON 15
#define PIN_TORPEDO_BUTTON 16

#define PIN_TEST_INTERRUPT 19

ButtonReader::ButtonReader() {
  pinMode(PIN_TEST_INTERRUPT, INPUT_PULLUP);

  pinMode(PIN_PHASER_BUTTON,INPUT_PULLUP);
  pinMode(PIN_TORPEDO_BUTTON,INPUT_PULLUP);
  pinMode(PIN_RED_ALERT,INPUT_PULLUP);
  pinMode(PIN_SPEED_DOWN,INPUT_PULLUP);
  pinMode(PIN_SPEED_UP,INPUT_PULLUP);
  pinMode(PIN_DEFLECTOR_MODE, INPUT_PULLUP);
  pinMode(PIN_PANEL_MODE,INPUT_PULLUP);
  pinMode(PIN_POWER_CYCLE, INPUT_PULLUP);
}

void ButtonReader::setupInterrupts() {
  PCintPort::attachInterrupt(PIN_TEST_INTERRUPT, testInterruptPinFall, FALLING); //RISING, FALLING and CHANGE
}

void ButtonReader::testInterruptPinFall() {
  EN1701A::buttonInterrupt = false;
  if (onButtonChangeDown(PIN_POWER_CYCLE, last_power_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_DEFLECTOR_MODE, last_deflector_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_TORPEDO_BUTTON, last_torpedo_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_PHASER_BUTTON, last_phaser_DebounceTime )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_SPEED_DOWN, last_speed_down_DebounceTime )) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_SPEED_UP, last_speed_up_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_RED_ALERT, last_red_alert_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }

  if (onButtonChangeDown(PIN_PANEL_MODE, last_panelmode_DebounceTime)) {
    EN1701A::buttonInterrupt = true;
    return;
  }
}

void ButtonReader::updateShipState_fromInterrupt(byte pinToTest)  {
   switch (pinToTest) {
     case PIN_POWER_CYCLE:
       EN1701A::b_power_cycle = !EN1701A::b_power_cycle;
       EN1701A::svWriteShipState(EN1701A::b_power_cycle, POWER_CHANGE);
       break;
     case PIN_RED_ALERT:
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
     case PIN_PHASER_BUTTON:
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
     case PIN_TORPEDO_BUTTON:
       EN1701A::svWriteShipState(true, TORPEDO);
       break;
     case PIN_DEFLECTOR_MODE:
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
     case PIN_SPEED_UP:
       if (EN1701A::b_warp_mode_on) {
         EN1701A::svWriteShipState(true, INCREASE_WARP_ENGINES);
       }
       else {
         EN1701A::svWriteShipState(true, INCREASE_IMPULSE_ENGINES);
       }
       break;
     case PIN_SPEED_DOWN:
       if (EN1701A::b_warp_mode_on) {
         EN1701A::svWriteShipState(true, DECREASE_WARP_ENGINES);
       }
       else {
         EN1701A::svWriteShipState(true, DECREASE_IMPULSE_ENGINES);
       }
       break;
     case PIN_PANEL_MODE:
       //EN1701A::svWriteShipState(buttonDown, SR_TORPEDO);
       break;
   }
}

boolean ButtonReader::onButtonChangeDown(byte pinToTest, unsigned long &debounceTarget ) {
  if ( digitalRead(pinToTest) == false ) {

    if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
      debounceTarget = millis();
      updateShipState_fromInterrupt(pinToTest);
      return true;
    }
  }
  return false;
}
