#include "Arduino.h"
#include <IRStateReader.h>

IRrecv *pReceiver;
IRdecodeNEC mDecoder;
unsigned long mTestStartMillis = 0;

IRStateReader::IRStateReader(int rp, unsigned int *currState, unsigned int *oldState) {
   pReceiver = new IRrecv(rp);
   pReceiver->enableIRIn(); // Start the receiver
   pCurrentShipState = currState;
   pOldShipState = oldState;
}

byte IRStateReader::getAudioIndex(){
  return mAudioIndex;
}

bool IRStateReader::updateShipStateViaIR() {

  bool isChanged = false;
  bool isRepeat = false;

  if (pReceiver->GetResults(&mDecoder))  {

     mDecoder.decode();    //Decode the data
     bool primary_systems_on = bitRead (*pCurrentShipState, PRIMARY_SYSTEMS);

     if ( mDecoder.value == 0 ){
       pReceiver->resume();
       return false;
     }

     Serial.println("");
     Serial.println("----DECODER BEFORE----");
     Serial.println(mDecoder.value);

     if ( mDecoder.value == 0xffffffff ) {
        switch (lastDecodedValue) {
          case 0xffd22d: //no repeat on POWER
          case 0xff22dd: //or torpedos
          case 0xff40bf: //or red alert
            lastDecodedValue = 0;
            break;
        }
        mDecoder.value = lastDecodedValue;
        isRepeat = true;
     }

     Serial.println(mDecoder.value);
     Serial.println("--- DECODE AFTER----");
     mTestStartMillis = millis();

     if ( primary_systems_on || mDecoder.value == 0xffd22d ) {
       isChanged = true;
       switch (mDecoder.value){
         //case STATE_PHASER_OFF:
        //    break;
         case 0xffd22d: //power on
            Serial.println("IRStateReader::POWER CHANGE");
            writeShipState(true, POWER_CHANGE);
            break;
         case 0xff12ed: //down
            Serial.println("IRStateReader::ENGINES DOWN");
            break;
          case 0xffa25d: //up
            Serial.println("IRStateReader::ENGINES UP");
            break;
          case 0xff22dd: //left
            Serial.println("IRStateReader::TORPEDO");
            writeShipState(true, TORPEDO);
            break;
          case 0xffe01f: //right
            Serial.println("IRStateReader::PHASER");
            if (!isRepeat){
              writeShipState(true, PHASER);
              mTestStartMillis = millis();
            }
            else {
              isChanged = false;
            }
            break;
         case 0xff40bf: //title
            Serial.println("IRStateReader::RED ALERT");
            mAudioIndex = AUDIO_INDEX_RED_ALERT;
            writeShipState(true, AUDIO_EFFECT);
            break;

         case 0xff30cf: //menu
            Serial.println("IRStateReader::P1 MSG");
            //mAudioEffect = WAV_P1_MESSAGE;
            writeShipState(true, AUDIO_EFFECT);
            break;

         case 0xff58a7: //STOP
            break;

         case 0xff52ad: //enter
            break;
         default:
            mDecoder.value = 0;
            isChanged = false;
            Serial.println("IRStateReader::ZERO");
            break;
       } // end switch
     } //end if primary_systems_on
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

bool IRStateReader::cleanTimeouts(unsigned long timerMillis){
    if ( bitRead(*pCurrentShipState, PHASER)){
       if ( timerMillis - mTestStartMillis >= 500 ) {
         writeShipState(false, PHASER);
         return true;
       }
    }

    return false;
}
