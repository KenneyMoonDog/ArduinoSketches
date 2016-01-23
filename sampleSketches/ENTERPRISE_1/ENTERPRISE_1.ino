#include <EN1701A.h>
#include <IRStateReader.h>
#include <ShipOperations.h>

unsigned int mCurrentShipState = 0;
unsigned int mPreviousShipState = 0;
unsigned long previousMillis = 0;

//----------
IRStateReader *pStateReader;
ShipOperations *pShipOperations;

void setup()
{
  Serial.begin(9600);
  
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_SR_SECTION_DATA, OUTPUT);  
  pinMode(PIN_SR_CLOCK, OUTPUT);
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_TORPEDO, OUTPUT);  
  pinMode(PIN_PHASER, OUTPUT);
  pinMode(PIN_PRIMARY_SYSTEMS, OUTPUT);
  pinMode(PIN_RUNNING_LIGHTS, OUTPUT);  
  pinMode(PIN_IMPULSE_DECK, OUTPUT);
  
  pStateReader = new IRStateReader(PIN_IR_RECEIVER, &mCurrentShipState, &mPreviousShipState);
  pShipOperations = new ShipOperations(&mCurrentShipState, &mPreviousShipState);

  pShipOperations->clearAll();
}
 
void loop() {
  
  unsigned long currentMillis = millis();
  if (pStateReader->updateShipStateViaIR()) {
    pShipOperations->ApplyShipLogic();
  }
  
  /*if (currentMillis - previousMillis >= 500) {
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;
    pShipOperations->cleanTimeouts();
  }*/
      
      /*analogWrite(PIN_SR_SECTION_ENABLE, pStateReader->getBrightness());

      leds = 0;
      bitSet(leds, bitToSet);
      updateShiftRegister();

      if ( bitToSet >= 7 || bitToSet <= 0) {
        direction = direction * -1;
      }

      bitToSet += direction;
  } */
}





