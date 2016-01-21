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

/*int IRStateReader::getLatchDelay(){
  return latchDelay;
}

int IRStateReader::getBrightness() {
  return brightness;
}*/

bool IRStateReader::updateShipStateViaIR() {

  bool isChanged = false;

  if (pReceiver->GetResults(&mDecoder))  {
     isChanged = true;
     mDecoder.decode();    //Decode the data

     if ( mDecoder.value == 0xffffffff ) {
        switch (lastDecodedValue) {
          case 0xffd22d: //no repeat on POWER
            lastDecodedValue = STATE_POWER_OFF;
            break;
        }
        mDecoder.value = lastDecodedValue;
     }

     Serial.println(mDecoder.value);

     switch (mDecoder.value){
       case STATE_POWER_OFF: //power off
         Serial.println("POWER OFF");
         if ( bitRead (*pCurrentShipState, PRIMARY_SYSTEMS) ) {
           updateShipState(SHUTDOWN);
         }

         break;
       case 0xffd22d: //power on
          Serial.println("POWER ON");
          //if PRIMARY_SYSTEMS is not on, is a full STARTUP
          if ( !bitRead (*pCurrentShipState, PRIMARY_SYSTEMS) ){
            updateShipState(STARTUP);
          }
          break;
       case 0xff12ed: //down
          Serial.println("BRIGHTER");
          //updateShipState(20);
          break;
        case 0xffa25d: //up
          Serial.println("DIMMER");
          //updateShipState(30);
          break;
        case 0xff22dd: //left
          Serial.println("FASTER");
          //updateShipState(40);
          break;
        case 0xffe01f: //right
          Serial.println("SLOWER");
          //updateShipState(50);
          break;
       default:
          mDecoder.value = 0;
          isChanged = false;
          break;
     } // end switch
     lastDecodedValue = mDecoder.value;
     pReceiver->resume(); //Restart the receiver
  } //end code received
  return isChanged;
}

void IRStateReader::updateShipState(unsigned int pinset ){
     *pOldShipState = *pCurrentShipState;
     bitSet(*pCurrentShipState, pinset);
}

/*void IRStateReader::updateShipStateViaIR() {

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
} */
