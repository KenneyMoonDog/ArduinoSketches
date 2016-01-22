#include "Arduino.h"
#include <IRStateReader.h>

IRrecv *pReceiver;
IRdecodeNEC mDecoder;

IRStateReader::IRStateReader(int rp, unsigned int *currState, unsigned int *oldState) {
   pReceiver = new IRrecv(rp);
   pReceiver->enableIRIn(); // Start the receiver
   pCurrentShipState = currState;
   pOldShipState = oldState;
}

bool IRStateReader::updateShipStateViaIR() {

  bool isChanged = false;

  if (pReceiver->GetResults(&mDecoder))  {
     isChanged = true;
     mDecoder.decode();    //Decode the data

     if ( mDecoder.value == 0xffffffff ) {
        switch (lastDecodedValue) {
          case 0xffd22d: //no repeat on POWER
          case 0xff22dd: //or torpedos
            lastDecodedValue = 0;
            break;
        }
        mDecoder.value = lastDecodedValue;
     }

     Serial.println(mDecoder.value);
     bool primary_systems_on = bitRead (*pCurrentShipState, PRIMARY_SYSTEMS);

     switch (mDecoder.value){
       /*case STATE_POWER_OFF: //power off
         Serial.println("POWER OFF");
         if ( bitRead (*pCurrentShipState, PRIMARY_SYSTEMS) ) {
           updateShipState(true, SHUTDOWN);
         }

         break; */
       case 0xffd22d: //power on
          Serial.println("POWER CHANGE");
          writeShipState(true, POWER_CHANGE);
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
          Serial.println("TORPEDO");
          if (primary_systems_on){
            writeShipState(true, TORPEDO);
          }
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

void IRStateReader::writeShipState(bool set, unsigned int pinset ){
    *pOldShipState = *pCurrentShipState;
   if (set) {
     bitSet(*pCurrentShipState, pinset);
   }
   else {
     bitClear(*pCurrentShipState, pinset);
   }
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
