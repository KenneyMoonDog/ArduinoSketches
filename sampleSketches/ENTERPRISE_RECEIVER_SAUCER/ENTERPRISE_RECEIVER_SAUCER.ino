#include <EN1701-REFIT.h>
#include <SERIAL_COMM.h>

int incomingByte = 0;   // for incoming serial data
unsigned long sectionData = 0;

//pin assignments
#define PIN_NAVIGATION_BEACON 17
#define PIN_NAVIGATION_FLASHER 18
#define PIN_SR_ENABLE 10
#define PIN_SR_CLOCK 11
#define PIN_SR_LATCH 12
#define PIN_SR_SECTION_DATA 13

#define PIN_CRYSTAL_R 6
#define PIN_CRYSTAL_G 5
#define PIN_CRYSTAL_B 3
#define PIN_PHASER 14
#define PIN_IMPULSE 9

//timer constants
//#define RECEIVER_INTERRUPT_FREQUENCY 100 //ms
//#define flashTimeOn 10
//#define flashTimeOff 11
//#define beaconTimeOn 25
//#define beaconTimeOff 35

//volatile unsigned long previousMillis = 0;
boolean bPowerOn = false;
//boolean bNavBeaconOn = false;
//boolean bNavFlasherOn = false;

byte currentCrystal_R = 0;
byte currentCrystal_G = 0;
byte currentCrystal_B = 0;

void setup() {
  pinMode(PIN_SR_ENABLE, OUTPUT);
  digitalWrite(PIN_SR_ENABLE,HIGH);
    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_BEACON, OUTPUT); 
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_SR_CLOCK, OUTPUT);  
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_SR_SECTION_DATA, OUTPUT);
  pinMode(PIN_CRYSTAL_R, OUTPUT);
  pinMode(PIN_CRYSTAL_G, OUTPUT);
  pinMode(PIN_CRYSTAL_B, OUTPUT);
  pinMode(PIN_PHASER, OUTPUT);
  
  updateSectionDataRegister();
  digitalWrite(PIN_SR_ENABLE,LOW);
  
  crystalLevel(50,50,50);
 
  //OCR0A = 0xAF;
  //TIMSK0 |= _BV(OCIE0A);
}

/*SIGNAL(TIMER0_COMPA_vect) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {  //execute any timed operations every INTERRRUPT FREQ ms
    // save the last time you did a repeatable item clear
    previousMillis = currentMillis;

    updateNavBeacon(bNavBeaconOn);
    //updateNavFlasher(bNavFlasherOn);
    randomSectionUpdate(bPowerOn);
  } //end if timer
} */

/*void updateNavBeacon(boolean bPowerOn){

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
}*/

/*void randomSectionUpdate(boolean bPowerOn) { //called every POLLING_FREQUENCY ms
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
}*/

void updateSectionDataRegister() {
  
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF));
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF00) >> 8);
   digitalWrite(PIN_SR_LATCH, HIGH);
}

void firePhaser(boolean bOn) {
  if (bOn){
    delay(300);
    digitalWrite(PIN_PHASER, HIGH);
  }
  else {
    digitalWrite(PIN_PHASER, LOW);
  }
}

/*void crystalLevel(byte R, byte G, byte B){
  analogWrite(PIN_CRYSTAL_R, 255-R);
  analogWrite(PIN_CRYSTAL_G, 255-G);
  analogWrite(PIN_CRYSTAL_B, 255-B);
}*/
void crystalLevel(byte R, byte G, byte B){

      bool oper_R = (R >= currentCrystal_R) ? 1:0;
      bool oper_G = (G >= currentCrystal_G) ? 1:0;
      bool oper_B = (B >= currentCrystal_B) ? 1:0;

      for (int increment=0; increment<=255; increment++){
         
         if ( currentCrystal_R != R ) {
           if ( oper_R ) {
             analogWrite(PIN_CRYSTAL_R, (255-(++currentCrystal_R)));
           }
           else {
             analogWrite(PIN_CRYSTAL_R, (255-(--currentCrystal_R)));
           }
         }

         if ( currentCrystal_G != G ) {
           if ( oper_G ) {
             analogWrite(PIN_CRYSTAL_G, (255-(++currentCrystal_G)));
           }
           else {
             analogWrite(PIN_CRYSTAL_G, (255-(--currentCrystal_G)));
           }
         }

         if ( currentCrystal_B != B ) {
           if ( oper_B ) {
             analogWrite(PIN_CRYSTAL_B, (255-(++currentCrystal_B)));
           }
           else {
             analogWrite(PIN_CRYSTAL_B, (255-(--currentCrystal_B)));
           }
         }
         delay(7);
      }  
}

void powerSaucerSectionUp(){
  bitSet(sectionData, SR_MAIN_POWER);
  
  /*updateSectionDataRegister();

  for (int section=0; section<8; section++){
    bitSet(sectionData, section);
    updateSectionDataRegister();
    delay(750);
  }*/
}

void powerSaucerSectionDown(){

  /*for (int section=0; section<8; section++){
    bitClear(sectionData, section);
    updateSectionDataRegister();
    delay(750);
  }*/
  
  bitClear(sectionData, SR_MAIN_POWER);
  updateSectionDataRegister();
}

void runShutdownSequence(){
  //bNavBeaconOn=false;
  //bNavFlasherOn=false;
  bPowerOn = false;
  powerSaucerSectionDown();
  crystalLevel(50,50,50);
}

void runStartUpSequence() {

  crystalLevel(255,64,0);
  delay(2000);
  powerSaucerSectionUp();

  //start nav lights
  bPowerOn = true;
  //bNavBeaconOn=true;
  //bNavFlasherOn=true;
  //start dome
  //start impulse engines
  //start warp engines 
  //fade in deflector
  delay(2000); 

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
          runShutdownSequence();
          break;
       case SERIAL_COMM_POWER_ON: //power on
          runStartUpSequence();
          break;
       case SERIAL_COMM_PHASER_ON:
          firePhaser(true);
          break;
       case SERIAL_COMM_PHASER_OFF:
          firePhaser(false);
          break;
       case SERIAL_COMM_NAV_BEACON_ON:
          digitalWrite(PIN_NAVIGATION_BEACON, HIGH);
          break;
       case SERIAL_COMM_NAV_BEACON_OFF:
          digitalWrite(PIN_NAVIGATION_BEACON, LOW);
          break;
       case SERIAL_COMM_NAV_FLASHER_ON:
          digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
          break;
       case SERIAL_COMM_NAV_FLASHER_OFF:
          digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
          break;
       default:
          switch (int(incomingByte/10) * 10){
            case SERIAL_COMM_IMPULSE_DRIVE:
               break;
            case SERIAL_COMM_WARP_DRIVE:
               break;
          }
     }
   }
}
