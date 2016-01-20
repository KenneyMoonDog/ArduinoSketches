#include "Arduino.h"
#include <IRStateReader.h>

IRrecv *pReceiver;
IRdecodeNEC mDecoder;

//IRStateReader::IRStateReader(int rp):mReceiver(rp) {
IRStateReader::IRStateReader(int rp, unsigned int *currState, unsigned int *oldState) {
   pReceiver = new IRrecv(rp);
   pReceiver->enableIRIn(); // Start the receiver
   pCurrentShipState = currState;
   pOldShipState = oldState;

   //bitSet(*pCurrentState, SECTION_2);
}

int IRStateReader::getLatchDelay(){
  return latchDelay;
}

int IRStateReader::getBrightness() {
  return brightness;
}

void IRStateReader::readIRCommand() {

  if (pReceiver->GetResults(&mDecoder))  {
     mDecoder.decode();    //Decode the data

     if ( mDecoder.value == 0xffffffff ) {
        switch (lastDecodedValue) {
          case 0xffd22d:
            lastDecodedValue = 0;
            break;
        }
        mDecoder.value = lastDecodedValue;
     }
     Serial.println(mDecoder.value);
     Serial.println(*pCurrentShipState);

     switch (mDecoder.value){
       case 0xffd22d: //power
          Serial.println("POWER");
          break;
       case 0xff12ed: //down
          Serial.println("BRIGHTER");
          if (brightness >= 5 ) {
            brightness -= 5;
          }
          break;
        case 0xffa25d: //up
          Serial.println("DIMMER");
          if (brightness <= 250){
            brightness += 5;
          }
          break;
        case 0xff22dd: //left
          Serial.println("FASTER");
          if (latchDelay >= 10){
            latchDelay -= 10;
          }
          break;
        case 0xffe01f: //right
          Serial.println("SLOWER");
          if (latchDelay <= 1000){
            latchDelay += 10;
          }
          break;
       default:
          mDecoder.value = 0;
          break;
     }
     lastDecodedValue = mDecoder.value;
     pReceiver->resume(); //Restart the receiver
  }
}
