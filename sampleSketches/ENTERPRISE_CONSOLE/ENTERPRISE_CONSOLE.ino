#include <EN1701-REFIT.h>
#include <ShipOperations.h>
#include <ButtonReader.h>

#define CONSOLE_POLLING_FREQUENCY 250 //ms
volatile unsigned long previousMillis = 0;

//IRStateReader *pStateReader;
ButtonReader *pButtonReader;
ShipOperations *pShipOperations;

byte          EN1701A::sbAudioIndex = 0;
unsigned long  EN1701A::suiCurrentShipState = 0;
unsigned long  EN1701A::suiPreviousShipState = 0;
volatile boolean EN1701A::buttonInterrupt = false;

void EN1701A::svWriteShipState(bool set, byte pinset )
{
  EN1701A::suiPreviousShipState = EN1701A::suiCurrentShipState;
  
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
  Serial.flush();
  
  pButtonReader->setupInterrupts();
  pShipOperations->clearAll();
  
  pShipOperations = new ShipOperations;
  pButtonReader = new ButtonReader;
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= CONSOLE_POLLING_FREQUENCY) {  //execute any timed operations every 250ms
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;; 
    pShipOperations->audioCheck();
  }
} 

void loop() {
  if ( EN1701A::buttonInterrupt ) {
    pShipOperations->ApplyShipLogic();
    EN1701A::buttonInterrupt = false;
  }
}

