#ifndef JButton_h
#define JButton_h

#include "Arduino.h"

class JButton {

  public:
   JButton();
   JButton(uint8_t pin, byte indx);
   JButton(uint8_t pin, byte indx, byte type);

   bool readButtonState();
   byte getButtonIndex();
   bool hasStateChanged();
   bool onButtonChange();
   uint8_t getPIN();

  private:
    int8_t PIN = 0;
    bool pressed = false;
    byte indexNum=0;
};
#endif
