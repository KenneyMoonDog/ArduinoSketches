#include "Arduino.h"
#include <IRStateReader.h>

IRrecv *pReceiver;
IRdecodeNEC mDecoder;
//unsigned long mTestStartMillis = 0;
//byte changeCounter=0;

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

     //Serial.println(F(""));
     //Serial.println(F("----DECODER BEFORE----"));
     //Serial.println(mDecoder.value);

     if ( mDecoder.value == 0xffffffff ) {
        /*switch (lastDecodedValue) {
          case 0xffd22d: //no repeat on POWER
          case 0xff22dd: //or torpedos
          case 0xff40bf: //or red alert
          case 0xff30cf: //or P1 Message
          case 0xff58a7: //or destruct
          case 0xff52ad: //or hull hit
          case 0xffe01f: //or phaser
            lastDecodedValue = 0;
            break;
        }*/
        lastDecodedValue = 0; //remove if the above switch is put back in
        mDecoder.value = lastDecodedValue;
        isRepeat = true;
     }

     //Serial.println(mDecoder.value);
     //Serial.println(F("--- DECODE AFTER----"));
     //mTestStartMillis = millis();

     if ( primary_systems_on || mDecoder.value == 0xffd22d ) {
       isChanged = true;
       switch (mDecoder.value){
         case 0xffd22d: //power on
            EN1701A::svWriteShipState(true, POWER_CHANGE);
            break;

         case 0xff12ed: //down
            break;

          case 0xffa25d: //up
            break;

          case 0xff22dd: //left
            EN1701A::svWriteShipState(true, SR_TORPEDO);
            break;

          case 0xffe01f: //right
            if ( bitRead(EN1701A::suiCurrentShipState, SR_PHASER) ) {
              EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
              EN1701A::svWriteShipState(false, SR_PHASER);
            }
            else {
              EN1701A::svWriteShipState(true, SR_PHASER);
            }
            break;

         case 0xff40bf: //title

            if ( EN1701A::sbAudioIndex == AUDIO_INDEX_RED_ALERT ) {
               EN1701A::sbAudioIndex = AUDIO_INDEX_CANCEL;
               EN1701A::svWriteShipState(false, AUDIO_EFFECT);
            }
            else {
              EN1701A::sbAudioIndex = AUDIO_INDEX_RED_ALERT;
              EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            }
            break;

         case 0xff30cf: //menu
            EN1701A::sbAudioIndex = AUDIO_INDEX_P1_MESSAGE;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;

         case 0xff58a7: //STOP
            EN1701A::sbAudioIndex = AUDIO_INDEX_DESTRUCT;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;

         case 0xff52ad: //enter
            EN1701A::sbAudioIndex = AUDIO_INDEX_HIT;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;

         case 0xff38c7:  //#1
            EN1701A::sbAudioIndex = AUDIO_INDEX_BTS1;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xff18e7:  //#2
            EN1701A::sbAudioIndex = AUDIO_INDEX_BTS2;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xffa857:  //#3
            EN1701A::sbAudioIndex = AUDIO_INDEX_BTS3;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xff48b7:  //#4
            EN1701A::sbAudioIndex = AUDIO_INDEX_BT4;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
         break;
         case 0xffc837:  //#5
            EN1701A::sbAudioIndex = AUDIO_INDEX_BT5;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xff28d7:  //#6
            EN1701A::sbAudioIndex = AUDIO_INDEX_BT6;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xffd827:  //#7
            EN1701A::sbAudioIndex = AUDIO_INDEX_BTS1;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xff7a85:  //#8
            EN1701A::sbAudioIndex = AUDIO_INDEX_BTS2;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xffaa55:  //#9
            EN1701A::sbAudioIndex = AUDIO_INDEX_BTS3;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
            break;
         case 0xff906f:  //#0
            EN1701A::sbAudioIndex = AUDIO_INDEX_BT4;
            EN1701A::svWriteShipState(true, AUDIO_EFFECT);
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

/*bool IRStateReader::executeTimedOperations(unsigned long currentMillis){ //called every 250ms

    bool rc = false;
    static byte changeCounter=0;
    static byte changeLimit = 2;

    //after some random elasped time, toggle one of the sections on or off at random
    if (changeCounter++ > changeLimit){
      changeCounter=0;
      changeLimit = random(1,5);
      EN1701A::suiCurrentShipState ^= (0x0001 << random(0,8));
      rc = true;
    }

    return rc;
}*/
