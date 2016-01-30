#include "Arduino.h"
#include <IRStateReader.h>

IRrecv *pReceiver;
IRdecodeNEC mDecoder;
unsigned long mTestStartMillis = 0;
byte changeCounter=0;

IRStateReader::IRStateReader(int rp) {
   pReceiver = new IRrecv(rp);
   pReceiver->enableIRIn(); // Start the receiver
}

bool IRStateReader::updateShipStateViaIR() {
  bool isChanged = false;
  bool isRepeat = false;

  if (pReceiver->GetResults(&mDecoder))  {

     mDecoder.decode();    //Decode the data
     bool primary_systems_on = bitRead (EN1701A::suiCurrentShipState, PRIMARY_SYSTEMS);

     if ( mDecoder.value == 0 ){
       pReceiver->resume();
       return false;
     }

     Serial.println(F(""));
     Serial.println(F("----DECODER BEFORE----"));
     Serial.println(mDecoder.value);

     if ( mDecoder.value == 0xffffffff ) {
        switch (lastDecodedValue) {
          case 0xffd22d: //no repeat on POWER
          case 0xff22dd: //or torpedos
          case 0xff40bf: //or red alert
          case 0xff30cf: //or P1 Message
            lastDecodedValue = 0;
            break;
        }
        mDecoder.value = lastDecodedValue;
        isRepeat = true;
     }

     Serial.println(mDecoder.value);
     Serial.println(F("--- DECODE AFTER----"));
     mTestStartMillis = millis();

     if ( primary_systems_on || mDecoder.value == 0xffd22d ) {
       isChanged = true;
       switch (mDecoder.value){
         //case STATE_PHASER_OFF:
        //    break;
         case 0xffd22d: //power on
            Serial.println(F("IRStateReader::POWER CHANGE"));
            EN1701A::svWriteShipState(true, POWER_CHANGE);
            break;
         case 0xff12ed: //down
            Serial.println(F("IRStateReader::ENGINES DOWN"));
            break;
          case 0xffa25d: //up
            Serial.println(F("IRStateReader::ENGINES UP"));
            break;
          case 0xff22dd: //left
            Serial.println(F("IRStateReader::TORPEDO"));
            EN1701A::svWriteShipState(true, TORPEDO);
            break;
          case 0xffe01f: //right
            Serial.println(F("IRStateReader::PHASER"));
            if (!isRepeat){
              Serial.println(F("IRStateReader::PHASER fire"));
              EN1701A::svWriteShipState(true, PHASER);
            }
            else {
              Serial.println(F("IRStateReader::PHASER repeat"));
              isChanged = false;
            }
            mTestStartMillis = millis();
            break;
         case 0xff40bf: //title

            if ( EN1701A::sbAudioIndex == AUDIO_INDEX_RED_ALERT ) {
              Serial.println(F("IRStateReader::RED ALERT OFF"));
               //setAudioIndex(AUDIO_INDEX_CANCEL);
               EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
            }
            else {
              Serial.println(F("IRStateReader::RED ALERT ON"));
              EN1701A::sbAudioIndex = AUDIO_INDEX_RED_ALERT;
              //setAudioIndex(AUDIO_INDEX_RED_ALERT);
            }
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;

         case 0xff30cf: //menu
            Serial.println(F("IRStateReader::P1 MSG"));
            EN1701A::sbAudioIndex = AUDIO_INDEX_P1_MESSAGE;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;

         case 0xff58a7: //STOP
            break;

         case 0xff52ad: //enter
            break;
         default:
            mDecoder.value = 0;
            isChanged = false;
            break;
       } // end switch
     } //end if primary_systems_on
     lastDecodedValue = mDecoder.value;
     pReceiver->resume(); //Restart the receiver

  } //end code received
  return isChanged;
}

bool IRStateReader::executeTimedOperations(unsigned long currentMillis){ //called every 250ms

    bool rc = false;
    if ( bitRead(EN1701A::suiCurrentShipState, PHASER)){
       if ( currentMillis - mTestStartMillis >= 500 ) {
         EN1701A::svWriteShipState(false, PHASER);
         //Serial.println("IRStateReader::interrupt PHASER");
         rc = true;
       }
    }

    if (changeCounter++ > 2){ //@40, 10 sec has elasped
      changeCounter=0;
      EN1701A::suiCurrentShipState &= 0xFF00;
      EN1701A::suiCurrentShipState |= (0xFF & rand() % 254 + 1);
      rc = true;
    }

    return rc;
}
