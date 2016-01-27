#include <EN1701A.h>
#include <IRStateReader.h>
#include <ShipOperations.h>

unsigned int mCurrentShipState = 0;
unsigned int mPreviousShipState = 0;
volatile unsigned long previousMillis = 0;

//----------
IRStateReader *pStateReader;
ShipOperations *pShipOperations;
char* mAudioEffects[]={"KLAX.WAV", "P1MSG.WAV", "TORP1.WAV", "SPZER1.WAV", "BPD1.WAV", "BPUP1.WAV"};
byte mAudioIndex;

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
  
  pStateReader = new IRStateReader(PIN_IR_RECEIVER, &mCurrentShipState, &mPreviousShipState, &mAudioIndex);
  pShipOperations = new ShipOperations(&mCurrentShipState, &mPreviousShipState, mAudioEffects, &mAudioIndex);

  pShipOperations->clearAll();

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 250) {
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;
    if (pStateReader->cleanTimeouts(currentMillis)){
      pShipOperations->ApplyShipLogic();
    }
  }
} 

void loop() {
  
  if (pStateReader->updateShipStateViaIR()) {
    pShipOperations->ApplyShipLogic();
  }
  
}





