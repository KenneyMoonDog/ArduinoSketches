#include <EN1701-REFIT.h>
#include <SERIAL_COMM.h>

//#include <EN1701-REFIT.h>
//#include <SERIAL_COMM.h>

int incomingByte = 0;   // for incoming serial data
unsigned long sectionData = 0;

//pin assignments
#define PIN_NAVIGATION_BEACON 4
#define PIN_NAVIGATION_FLASHER 8
#define PIN_SR_CLOCK 11
#define PIN_SR_LATCH 12
#define PIN_SR_SECTION_DATA 13

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 100 //ms
#define flashPeriod 30
#define flashDuration 1
#define beaconPeriod 10

volatile unsigned long previousMillis = 0;
boolean bPowerOn = false;
boolean bNavBeaconOn = false;
boolean bNavFlasherOn = false;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_BEACON, OUTPUT); 
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_SR_CLOCK, OUTPUT);  
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_SR_SECTION_DATA, OUTPUT);
 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {  //execute any timed operations every 250ms
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;

    updateNavBeacon(bPowerOn);
    updateNavFlasher(bPowerOn);
    randomSectionUpdate(bPowerOn);
    updateSectionDataRegister();
  } //end if timer
} 

void updateNavBeacon(boolean bPowerOn){

  static byte changeBeaconCounter=0;

  if (bPowerOn) {
     if (changeBeaconCounter++ > beaconPeriod){
        changeBeaconCounter=0;
        if ( bNavBeaconOn ){
          bNavBeaconOn = false;
          digitalWrite(PIN_NAVIGATION_BEACON, LOW);
        }
        else {
          bNavBeaconOn=true;
          digitalWrite(PIN_NAVIGATION_BEACON, HIGH);
        }
     }
  }
  else {  //power is off
    if ( bNavBeaconOn ){
      bNavBeaconOn = false;
      digitalWrite(PIN_NAVIGATION_BEACON, LOW);     
    }
  }
}

void updateNavFlasher(boolean bPowerOn){

  static byte flashPeriodCounter=0;
  static byte flashDurationCounter=0;

  if (bPowerOn) {
     if (flashPeriodCounter++ > flashPeriod){
        flashPeriodCounter=0;
        digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
        bNavFlasherOn = true;
     }

     if ( bNavFlasherOn && (flashDurationCounter++ > flashDuration)){ 
        flashDurationCounter=0;
        digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
        bNavFlasherOn = false;  
     }
  }
  else {  //power is off
    if ( bNavFlasherOn ){
      bNavFlasherOn = false;
      digitalWrite(PIN_NAVIGATION_FLASHER, LOW);     
    }
  }
}

void randomSectionUpdate(boolean bPowerOn) { //called every POLLING_FREQUENCY ms
    static byte changeCounter=0; //increments every INTERRUPT_FREQ duration
    static byte changeLimit=50; //INTERRUPT_FREQ*changeLimit = ms duration

    if (bPowerOn) {
    //after some random elasped time, toggle one of the sections on or off at random
      if (changeCounter++ > changeLimit){
        changeCounter=0;
        changeLimit = random(20,80);
        sectionData ^= (0x0001 << random(0,8));
      //updateSectionDataRegister();
      }
    }
    else {
      if (sectionData != 0 ){
         sectionData = 0;
         updateSectionDataRegister();
      }
    }
}

void updateSectionDataRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF));
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF00) >> 8);
   digitalWrite(PIN_SR_LATCH, HIGH);
}

void fireTorpedo() {
  bitSet(sectionData, SR_TORPEDO);
  updateSectionDataRegister();
  delay(200);
  bitClear(sectionData, SR_TORPEDO);
  updateSectionDataRegister();  
}

void firePhaser(boolean bOn) {
  if (bOn){
    bitSet(sectionData, SR_PHASER);
  }
  else {
    bitClear(sectionData, SR_PHASER);
  }
  updateSectionDataRegister();
}

void loop() {

   if (Serial.available() > 0) {
     // read the incoming byte:
     incomingByte = Serial.read();

     switch (incomingByte) {
       case SERIAL_COMM_POWER_OFF: 
          bPowerOn = false;
          break;
       case SERIAL_COMM_POWER_ON: //power on
          bPowerOn = true;
          break;
       case SERIAL_COMM_TORPEDO:
          fireTorpedo();
          break;
       case SERIAL_COMM_PHASER_ON:
          firePhaser(true);
          break;
       case SERIAL_COMM_PHASER_OFF:
          firePhaser(false);
          break;
     }
   }
}
