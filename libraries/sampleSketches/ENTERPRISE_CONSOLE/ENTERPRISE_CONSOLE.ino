#include <EN1701-REFIT.h>
#include <ShipOperations.h>
#include <ButtonReader.h>

#define CONSOLE_POLLING_FREQUENCY 250 //ms
volatile unsigned long previousMillis = 0;

ButtonReader *pButtonReader;
ShipOperations *pShipOperations;

volatile byte          EN1701A::sbAudioIndex = 0;
volatile unsigned long  EN1701A::suiCurrentShipState = 0;
volatile unsigned long  EN1701A::suiPreviousShipState = 0;
volatile boolean EN1701A::buttonInterrupt = false;
volatile boolean EN1701A::b_warp_mode_on = false;
volatile boolean EN1701A::b_red_alert_on = false;
volatile boolean EN1701A::b_phaser_on = false;
volatile boolean EN1701A::b_power_cycle = false;

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
  
  pShipOperations = new ShipOperations;
  pButtonReader = new ButtonReader;

  pButtonReader->setupInterrupts();
  pShipOperations->clearAll();
  
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
