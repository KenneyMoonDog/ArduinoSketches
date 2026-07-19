#include "JButton.h"

    JButton::JButton(uint8_t pin, byte indx) {
      PIN=pin;
      pinMode(PIN, INPUT_PULLUP);
      indexNum=indx;
    }

    JButton::JButton(uint8_t pin, byte indx, byte type) {
      PIN=pin;
      pinMode(PIN, type);
      indexNum=indx;
    }

    bool JButton::readButtonState(){
       return pressed;
    }

    byte JButton::getButtonIndex(){
      return indexNum;
    }

    bool JButton::hasStateChanged() {  //return true if the value of the button has changed

       bool currentButtonState = pressed;
       pressed = this->onButtonChange();

       if (currentButtonState != pressed){
         return true;
       }
       else {
         return false;
       }
    }

    bool JButton::onButtonChange(){  //read the digital pin for its value.  Note this assumes the signal is debounced properly
      delay(20);
      if ( digitalRead(PIN) == LOW ){
        return true;
      }
      else {
        return false;
      }
    }

    uint8_t JButton::getPIN() {
      return PIN;
    }
