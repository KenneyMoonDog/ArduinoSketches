#include <EN1701A.h>
#include <IRStateReader.h>

unsigned int mCurrentShipState = 0;
unsigned int mPreviousShipState = 0;

unsigned int mCurrentPinState = 0;
unsigned int mPreviousPinState = 0;

byte leds = 0;
byte bitToSet = 1;
byte direction = 1;

//----------
unsigned long previousMillis = 0;
IRStateReader *pStateReader;

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
}
 
void loop() {

  unsigned long currentMillis = millis();
  pStateReader->readIRCommand();
  
  if (currentMillis - previousMillis >= pStateReader->getLatchDelay()) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      analogWrite(PIN_SR_SECTION_ENABLE, pStateReader->getBrightness());

      leds = 0;
      bitSet(leds, bitToSet);
      updateShiftRegister();

      if ( bitToSet >= 7 || bitToSet <= 0) {
        direction = direction * -1;
      }

      bitToSet += direction;
  }
}

void updateShiftRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, leds);
   digitalWrite(PIN_SR_LATCH, HIGH);
}



