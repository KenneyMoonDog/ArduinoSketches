#include <EN1701-REFIT.h>
#include <SERIAL_COMM.h>

int incomingByte = 0;   // for incoming serial data
unsigned long sectionData = 0;

//pin assignments
#define PIN_NAVIGATION_BEACON 4
#define PIN_NAVIGATION_FLASHER 8
#define PIN_SR_CLOCK 11
#define PIN_SR_LATCH 12
#define PIN_SR_SECTION_DATA 13
#define PIN_SR_ENABLE 14
#define PIN_DEFLECTOR_R 6
#define PIN_DEFLECTOR_G 5
#define PIN_DEFLECTOR_B 3
//#define PIN_WARP_ENGINE 5
//#define PIN_IMPULSE_ENGINE 4
//#define PIN_DILITHIUM_CRYSTAL 10
#define PIN_PHOTON_TORPEDO 9 

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 100 //ms
#define flashTimeOn 10
#define flashTimeOff 11
#define beaconTimeOn 25
#define beaconTimeOff 35

volatile unsigned long previousMillis = 0;
boolean bPowerOn = false;
boolean bNavBeaconOn = false;
boolean bNavFlasherOn = false;

void setup() {
  pinMode(PIN_SR_ENABLE, OUTPUT);
  digitalWrite(PIN_SR_ENABLE,HIGH);
    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_BEACON, OUTPUT); 
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_SR_CLOCK, OUTPUT);  
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_SR_SECTION_DATA, OUTPUT);
  pinMode(PIN_DEFLECTOR_R, OUTPUT);
  pinMode(PIN_DEFLECTOR_G, OUTPUT);
  pinMode(PIN_DEFLECTOR_B, OUTPUT);
  //pinMode(PIN_DILITHIUM_CRYSTAL, OUTPUT); 
  pinMode(PIN_PHOTON_TORPEDO, OUTPUT);
  
  updateSectionDataRegister();
  digitalWrite(PIN_SR_ENABLE,LOW);
 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {  //execute any timed operations every INTERRRUPT FREQ ms
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;

    updateNavBeacon(bNavBeaconOn);
    //updateNavFlasher(bNavFlasherOn);
    randomSectionUpdate(bPowerOn);
  } //end if timer
} 

void updateNavBeacon(boolean bPowerOn){

  static byte bCounter=0;
  static boolean bBeaconOn=false;
  static boolean bFlasherOn=false;
  
  if (bPowerOn) {
     if (bCounter >= beaconTimeOn && bCounter < beaconTimeOff ){
        if ( !bBeaconOn ) {
          digitalWrite(PIN_NAVIGATION_BEACON, HIGH);
          bBeaconOn = true;
        }
     }

     if (bBeaconOn && bCounter >= beaconTimeOff ){
        digitalWrite(PIN_NAVIGATION_BEACON, LOW);
        bCounter = -1;
        bBeaconOn = false;
     } 

     if (bCounter >= flashTimeOn && bCounter < flashTimeOff ){
        if ( !bFlasherOn ){
          digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
          bFlasherOn = true;
        }
     }

     if ( bFlasherOn && bCounter >= flashTimeOff ) {
        digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
        bFlasherOn = false;  
     }  
     bCounter++;
  }
  else {  //power is off
    if ( bBeaconOn ){
      bBeaconOn = false;
      digitalWrite(PIN_NAVIGATION_BEACON, LOW);
    }
    if ( bFlasherOn ){
      bFlasherOn = false;
      digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
    }
  }
}

/*void updateNavFlasher(boolean bPowerOn){

  static byte flashPeriodCounter=0;
  static byte flashDurationCounter=0;
  static boolean bFlasherState=false;

  if (bPowerOn) {
     if (flashPeriodCounter++ > flashPeriod){
        flashPeriodCounter=0;
        digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
        bFlasherState = true;
     }

     if ( bFlasherState && (flashDurationCounter++ > flashDuration)){ 
        flashDurationCounter=0;
        digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
        bFlasherState = false;  
     }
  }
  else {  //power is off
    if ( bFlasherState ){
      bFlasherState = false;
      digitalWrite(PIN_NAVIGATION_FLASHER, LOW);     
    }
  }
}*/

void randomSectionUpdate(boolean bPowerOn) { //called every POLLING_FREQUENCY ms
   static byte changeCounter=0; //increments every INTERRUPT_FREQ duration
   static byte changeLimit=50; //INTERRUPT_FREQ*changeLimit = ms duration

   if (bPowerOn) {
    //after some random elasped time, toggle one of the sections on or off at random
      if (changeCounter++ > changeLimit){
        changeCounter=0;
        changeLimit = random(20,80);
        sectionData ^= (0x0001 << random(0,6));
      }
      updateSectionDataRegister();
   }
}

void updateSectionDataRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF));
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF00) >> 8);
   digitalWrite(PIN_SR_LATCH, HIGH);
}

/*void fireTorpedo() {
  bitSet(sectionData, SR_TORPEDO);
  updateSectionDataRegister();
  delay(200);
  bitClear(sectionData, SR_TORPEDO);
  updateSectionDataRegister();  
}*/

void fireTorpedo() {
   //for (int brightness=0; brightness<=10; brightness+=1){
   //   analogWrite(PIN_PHOTON_TORPEDO, brightness);
   //   delay(5);
   //}  
   analogWrite(PIN_PHOTON_TORPEDO, 10);
   delay(100);
   digitalWrite(PIN_PHOTON_TORPEDO, HIGH);
   delay(250);
   
   for (int brightness=10; brightness>=0; brightness-=1){
      analogWrite(PIN_PHOTON_TORPEDO, brightness);
      delay(100);
   }  
}

void firePhaser(boolean bOn) {
  if (bOn){
    delay(300);
    bitSet(sectionData, SR_PHASER);
  }
  else {
    bitClear(sectionData, SR_PHASER);
  }
  updateSectionDataRegister();
}

void deflectorOn(boolean bOn){
   if (bOn) {
      for (int brightness=0; brightness<=100; brightness+=5){
         analogWrite(PIN_DEFLECTOR_R, brightness);
         analogWrite(PIN_DEFLECTOR_G, brightness);
         analogWrite(PIN_DEFLECTOR_B, brightness);
         delay(50);
      }
   }
   else {
      for (int brightness=100; brightness>=0; brightness-=5){
         analogWrite(PIN_DEFLECTOR_R, brightness);
         analogWrite(PIN_DEFLECTOR_G, brightness);
         analogWrite(PIN_DEFLECTOR_B, brightness);
         delay(50);
      }
   }
}

void powerSaucerSectionUp(){
  bitSet(sectionData, SR_MAIN_POWER);
  updateSectionDataRegister();

  for (int section=0; section<8; section++){
    bitSet(sectionData, section);
    updateSectionDataRegister();
    delay(750);
  }
}

void powerSaucerSectionDown(){

  for (int section=0; section<8; section++){
    bitClear(sectionData, section);
    updateSectionDataRegister();
    delay(750);
  }
  
  bitClear(sectionData, SR_MAIN_POWER);
  updateSectionDataRegister();
}

void runShutdownSequence(){
  bNavBeaconOn=false;
  bNavFlasherOn=false;
  bPowerOn = false;
  powerSaucerSectionDown();
  deflectorOn( false );
}

void runStartUpSequence() {

  powerSaucerSectionUp();

  //start nav lights
  bPowerOn = true;
  bNavBeaconOn=true;
  bNavFlasherOn=true;
  //start dome
  //start impulse engines
  //start warp engines 
  //fade in deflector
  delay(2000); 
  deflectorOn( true );
  //start running lights
  //start random section updates
  //increase warp engines
}

void loop() {

   if (Serial.available() > 0) {
     // read the incoming byte:
     incomingByte = Serial.read();

     switch (incomingByte) {
       case SERIAL_COMM_POWER_OFF: 
          Serial.write(SERIAL_COMM_POWER_OFF);
          runShutdownSequence();
          break;
       case SERIAL_COMM_POWER_ON: //power on
          Serial.write(SERIAL_COMM_POWER_ON);
          runStartUpSequence();
          break;
       case SERIAL_COMM_TORPEDO:
          fireTorpedo();
          break;
       case SERIAL_COMM_PHASER_ON:
          Serial.write(SERIAL_COMM_PHASER_ON);
          //firePhaser(true);
          break;
       case SERIAL_COMM_PHASER_OFF:
          Serial.write(SERIAL_COMM_PHASER_OFF);
          //firePhaser(false);
          break;
     }
   }
}
