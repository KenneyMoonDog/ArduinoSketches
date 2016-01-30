#include <EN1701A.h>
#include <IRStateReader.h>
#include <ShipOperations.h>

volatile unsigned long previousMillis = 0;

//----------
IRStateReader *pStateReader;
ShipOperations *pShipOperations;

byte          EN1701A::sbAudioIndex = 0;
unsigned int  EN1701A::suiCurrentShipState = 0;
unsigned int  EN1701A::suiPreviousShipState = 0;

void EN1701A::svWriteShipState(bool set, unsigned int pinset )
{
  suiPreviousShipState = suiCurrentShipState;
  if (set) {
    bitSet(suiCurrentShipState, pinset);
  }
  else {
    bitClear(suiCurrentShipState, pinset);
  }
}

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
  
  pStateReader = new IRStateReader(PIN_IR_RECEIVER);
  pShipOperations = new ShipOperations();

  pShipOperations->clearAll();

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 250) {  //execute any timed operations every 250ms
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;
    if (pStateReader->executeTimedOperations(currentMillis)){
      pShipOperations->ApplyShipLogic();
    }
  }
} 

void loop() {
  if (pStateReader->updateShipStateViaIR()) {
    pShipOperations->ApplyShipLogic();
  } 
}





