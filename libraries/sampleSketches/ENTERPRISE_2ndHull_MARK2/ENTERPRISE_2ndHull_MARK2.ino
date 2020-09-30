#include <EN1701-REFIT_MK2.h>
#include <SERIAL_COMM_MK2.h>

byte incomingByte = 0;   // for incoming serial data
byte stateIn = 0;

unsigned long saucerSectionData = 0;
byte newImpulseLevel=0;

//unsigned long hullSectionData = 0;

#define SAUCER_SECTION_1 1
#define SAUCER_SECTION_2 2
#define SAUCER_SECTION_3 3
#define SAUCER_SECTION_4 4
#define SAUCER_SECTION_5 5
#define SAUCER_SECTION_6 6
#define SAUCER_SECTION_BRIDGE 8
#define SAUCER_SECTION_LOUNGE 7

#define PIN_NECK_LIGHTING 3
#define PIN_ARBORITUM 4
#define PIN_PHOTON_TORPEDO_2 5
#define PIN_PHOTON_TORPEDO_1 6
#define PIN_AFT_LIGHTS 8
#define PIN_DEFLECTOR_R 9
#define PIN_DEFLECTOR_G 11
#define PIN_DEFLECTOR_B 10
#define PIN_NAVIGATION_FLASHER 12
#define PIN_RED_ALERT 13

#define PIN_FLOOD_1 14
#define PIN_HANGER 15
#define PIN_DOWN_BELOW 16
#define PIN_STARBOARD_LIGHTS 17
#define PIN_PORT_LIGHTS 18
#define PIN_FLOOD_2 19

//timer constants
#define RECEIVER_INTERRUPT_FREQ_FLASHER 100 //ms
#define RECEIVER_INTERRUPT_FREQ_BEACON 10 //ms
#define RECEIVER_INTERRUPT_FREQ_RED_ALERT 10 //ms
#define flashTimeOn 10
#define flashTimeOff 11
#define beaconTimeOn 25
#define beaconTimeOff 35
#define redAlertTimeOff 100
#define redAlertTimeEnd 180

byte sectionSignal[] = {0,0};
volatile unsigned long flasherPreviousMillis = 0;
volatile unsigned long beaconPreviousMillis = 0;
volatile unsigned long redAlertPreviousMillis = 0;

boolean bPowerOn = false;
volatile boolean bNavBeaconOn = false;
volatile boolean bNavFlasherOn = false;
volatile boolean bRedAlertOn = false;
volatile boolean bRedLedOn = false;
boolean fullImpulse = false;
boolean testMode = false;

byte crystalSignal[] = {0,0,0,0};
byte nacelleSignal[] = {0,0,0,0};
volatile byte newDeflectorRGB[] = {0,0,0};
volatile byte oldDeflectorRGB[] = {0,0,0};
byte impulseLevelSignal[] = {SERIAL_COMM_IMPULSE_DRIVE, 0};

byte torpedo_tube = PIN_PHOTON_TORPEDO_1;

void setup() {
  //pinMode(PIN_SR_ENABLE, OUTPUT);
  //digitalWrite(PIN_SR_ENABLE,HIGH);
    
  Serial.begin(9600);
  Serial.flush();

  pinMode(PIN_NECK_LIGHTING, OUTPUT);
  pinMode(PIN_ARBORITUM, OUTPUT);
  pinMode(PIN_PHOTON_TORPEDO_1, OUTPUT);
  pinMode(PIN_PHOTON_TORPEDO_2, OUTPUT);
  pinMode(PIN_DEFLECTOR_R, OUTPUT);
  pinMode(PIN_DEFLECTOR_G, OUTPUT);
  pinMode(PIN_DEFLECTOR_B, OUTPUT);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);
  pinMode(PIN_RED_ALERT, OUTPUT);
  pinMode(PIN_FLOOD_1, OUTPUT);
  pinMode(PIN_HANGER, OUTPUT);
  pinMode(PIN_DOWN_BELOW, OUTPUT);
  pinMode(PIN_STARBOARD_LIGHTS, OUTPUT);
  pinMode(PIN_PORT_LIGHTS, OUTPUT);
  pinMode(PIN_FLOOD_2, OUTPUT);
  pinMode(PIN_AFT_LIGHTS, OUTPUT);

  analogWrite(PIN_DEFLECTOR_R,255);
  analogWrite(PIN_DEFLECTOR_G,255);
  analogWrite(PIN_DEFLECTOR_B,255);
 
  updateSaucerSectionDataRegister();
 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - flasherPreviousMillis >= RECEIVER_INTERRUPT_FREQ_FLASHER) {  //execute any timed operations every INTERRRUPT FREQ ms
    // save the last time you did a repeatable item clear
    flasherPreviousMillis = currentMillis;
    updateNavBeacon(bNavBeaconOn);
    //randomSaucerSectionUpdate(bPowerOn);
  } //end if timer

  if (currentMillis - beaconPreviousMillis >= RECEIVER_INTERRUPT_FREQ_BEACON) {
    beaconPreviousMillis = currentMillis;
    checkDeflectorLevel();
  }

  if (currentMillis - redAlertPreviousMillis >= RECEIVER_INTERRUPT_FREQ_RED_ALERT) {
    redAlertPreviousMillis = currentMillis;
    updateRedAlert();
  }
} 

void checkDeflectorLevel(){

   bool oper_R = (oldDeflectorRGB[0] >= newDeflectorRGB[0]) ? 0:1;
   bool oper_G = (oldDeflectorRGB[1] >= newDeflectorRGB[1]) ? 0:1;
   bool oper_B = (oldDeflectorRGB[2] >= newDeflectorRGB[2]) ? 0:1;      
         
   if ( oldDeflectorRGB[0] != newDeflectorRGB[0] ) {
     if ( oper_R ) {
       analogWrite(PIN_DEFLECTOR_R, (255-(++oldDeflectorRGB[0])));
     }
     else {
       analogWrite(PIN_DEFLECTOR_R, (255-(--oldDeflectorRGB[0])));
     }
   }

   if ( oldDeflectorRGB[1] != newDeflectorRGB[1] ) {
      if ( oper_G ) {
         analogWrite(PIN_DEFLECTOR_G, (255-(++oldDeflectorRGB[1])));
      }
      else {
         analogWrite(PIN_DEFLECTOR_G, (255-(--oldDeflectorRGB[1])));
      }
   }

   if ( oldDeflectorRGB[2] != newDeflectorRGB[2] ) {
      if ( oper_B ) {
         analogWrite(PIN_DEFLECTOR_B, (255-(++oldDeflectorRGB[2])));
      }
      else {
         analogWrite(PIN_DEFLECTOR_B, (255-(--oldDeflectorRGB[2])));
      }
   }
}

void setDeflector(byte color[]) {
      newDeflectorRGB[0] = color[0];
      newDeflectorRGB[1] = color[1];
      newDeflectorRGB[2] = color[2];
}

void updateRedAlert() {
  static volatile int bREACounter=0;

  if (bRedAlertOn) {
     if (bREACounter < redAlertTimeOff){
        if ( !bRedLedOn ) {
          digitalWrite(PIN_RED_ALERT, true);
          digitalWrite(PIN_HANGER, false);
          bRedLedOn = true;
        }
     } 
         
     if (bREACounter >= redAlertTimeOff){
        if (bRedLedOn){
          digitalWrite(PIN_RED_ALERT, false);
          digitalWrite(PIN_HANGER, true);
          bRedLedOn = false;
        }
     }

     if (bREACounter > redAlertTimeEnd ){
        bREACounter=0;
     }

     bREACounter++;
  }
  else {  //no red alert
    if ( bRedLedOn ){
      bRedLedOn = false;
      digitalWrite(PIN_RED_ALERT, false);
      digitalWrite(PIN_HANGER, true);
    }
  }
}

void updateNavBeacon(boolean bPowerOn){

  static volatile byte bCounter=0;
  static volatile boolean bBeaconOn=false;
  static volatile boolean bFlasherOn=false;
  
  if (bPowerOn) {
     if (bCounter >= beaconTimeOn && bCounter < beaconTimeOff ){
        if ( !bBeaconOn ) {
          Serial.write(SERIAL_COMM_NAV_BEACON_ON);
          bBeaconOn = true;
        }
     }

     if (bBeaconOn && bCounter >= beaconTimeOff ){
        Serial.write(SERIAL_COMM_NAV_BEACON_OFF);
        bCounter = -1;
        bBeaconOn = false;
     } 

     if (bCounter >= flashTimeOn && bCounter < flashTimeOff ){
        if ( !bFlasherOn ){
          digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
          Serial.write(SERIAL_COMM_NAV_FLASHER_ON);
          bFlasherOn = true;
        }
     }

     if ( bFlasherOn && bCounter >= flashTimeOff ) {
        digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
        Serial.write(SERIAL_COMM_NAV_FLASHER_OFF);
        bFlasherOn = false;  
     }  
     bCounter++;
  }
  else {  //power is off
    if ( bBeaconOn ){
      bBeaconOn = false;
      Serial.write(SERIAL_COMM_NAV_BEACON_OFF);
    }
    if ( bFlasherOn ){
      bFlasherOn = false;
      digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
      Serial.write(SERIAL_COMM_NAV_FLASHER_OFF);
    }
  }
}

/*void toggleSaucerSection(byte section){
    int sectionNumber = section - 1;
    if (bPowerOn) {
       if (bitRead(saucerSectionData, sectionNumber) == 1){
          bitClear(saucerSectionData, sectionNumber);
       }
       else {
          bitSet(saucerSectionData, sectionNumber);
       }
       updateSaucerSectionDataRegister();
    }  
}*/

/*void randomSaucerSectionUpdate(boolean bPowerOn) { //called every POLLING_FREQUENCY ms
   static byte changeCounter=0; //increments every INTERRUPT_FREQ duration
   static byte changeLimit=50; //INTERRUPT_FREQ*changeLimit = ms duration

   if (bPowerOn) {
    //after some random elasped time, toggle one of the sections on or off at random
      if (changeCounter++ > changeLimit){
        changeCounter=0;
        changeLimit = random(20,80);
        saucerSectionData ^= (0x0001 << random(0,6));
      }
      updateSaucerSectionDataRegister();
   }
}*/

/*void updateHullSectionDataRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (hullSectionData & 0xFF));
   //shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF00) >> 8);
   digitalWrite(PIN_SR_LATCH, HIGH);
}*/

/*void sectionPower( byte num, ... ){
  va_list arguments;                     
  va_start ( arguments, num );

  bool set = va_arg(arguments, bool);
  int delayReturn = va_arg(arguments, int);
  
  for (byte count = 0; count<(num-2); count++){
    byte bitToSet = va_arg(arguments, byte) - 1;
    if (set){
      bitSet(saucerSectionData, bitToSet);
    } 
    else {
      bitSet(saucerSectionData, bitToSet);
    }
  }
  
  va_end(arguments);
  updateSaucerSectionDataRegister();
  delay(delayReturn);
}*/

/*void decreaseImpulseDrive() {
  impulseLevelSignal[1]-=50;
  
  if (impulseLevelSignal[1] < 0){
    impulseLevelSignal[1]=0;
  }
  Serial.write(impulseLevelSignal, 2);
}

void increaseImpulseDrive() {
  impulseLevelSignal[1]+=50;
  
  if (impulseLevelSignal[1] > 255){
    impulseLevelSignal[1]=255;
  }
  
  Serial.write(impulseLevelSignal, 2);
}*/

void setImpulseDrive(byte level) {
   impulseLevelSignal[1] = level;
   Serial.write(impulseLevelSignal, 2);
}

void fireTorpedo() {
   analogWrite(torpedo_tube, 10);
   delay(100);
   digitalWrite(torpedo_tube, HIGH);
   delay(250);
   
   for (int brightness=10; brightness>=0; brightness-=1){
      analogWrite(torpedo_tube, brightness);
      delay(120);
   }  

   if ( torpedo_tube == PIN_PHOTON_TORPEDO_1 ){
      torpedo_tube = PIN_PHOTON_TORPEDO_2;
   }
   else {
      torpedo_tube = PIN_PHOTON_TORPEDO_1;
   }
}

void delayedSectionUpdate(int returnDelay){
  updateSaucerSectionDataRegister();
  delay(returnDelay);
}

void powerSaucerSectionUp(){

  bitSet(saucerSectionData, (SAUCER_SECTION_LOUNGE-1));
  delayedSectionUpdate(1000);
  bitSet(saucerSectionData, (SAUCER_SECTION_3 - 1));
  bitSet(saucerSectionData, (SAUCER_SECTION_4 - 1));
  delayedSectionUpdate(1000);
  bitSet(saucerSectionData, (SAUCER_SECTION_2 - 1));
  bitSet(saucerSectionData, (SAUCER_SECTION_5 - 1));
  delayedSectionUpdate(1000);
  bitSet(saucerSectionData, (SAUCER_SECTION_1 - 1));
  bitSet(saucerSectionData, (SAUCER_SECTION_6 - 1));
  delayedSectionUpdate(1000);
  delayedSectionUpdate(1000);
  bitSet(saucerSectionData, (SAUCER_SECTION_BRIDGE - 1));
  delayedSectionUpdate(0);
}

void powerSaucerSectionDown(){
  bitClear(saucerSectionData, (SAUCER_SECTION_BRIDGE - 1));
  delayedSectionUpdate(1000);
  bitClear(saucerSectionData, (SAUCER_SECTION_1 - 1));
  bitClear(saucerSectionData, (SAUCER_SECTION_6 - 1));
  delayedSectionUpdate(1000);
  bitClear(saucerSectionData, (SAUCER_SECTION_3 - 1));
  bitClear(saucerSectionData, (SAUCER_SECTION_4 - 1));
  delayedSectionUpdate(1000);
  bitClear(saucerSectionData, (SAUCER_SECTION_2 - 1));
  bitClear(saucerSectionData, (SAUCER_SECTION_5 - 1));
  delayedSectionUpdate(1000);
  bitClear(saucerSectionData, (SAUCER_SECTION_LOUNGE-1));
  delayedSectionUpdate(1000);
}

void runShutdownSequence(){
  bNavBeaconOn=false;
  bNavFlasherOn=false;
  bPowerOn = false;
  bRedAlertOn = false;
}

void runStartUpSequence() {
  bPowerOn = true;
 
  //start nav lights
  bNavBeaconOn=true;
  bNavFlasherOn=true;
}

void setCrystal(byte pRGB[]) {
   crystalSignal[0] = SERIAL_COMM_CRYSTAL_COLOR;
   crystalSignal[1] = pRGB[0];
   crystalSignal[2] = pRGB[1];
   crystalSignal[3] = pRGB[2];
   Serial.write(crystalSignal, 4); 
}

void setNacelles(byte pRGB[]) {
   nacelleSignal[0] = SERIAL_COMM_NACELLE_COLOR;
   nacelleSignal[1] = pRGB[0];
   nacelleSignal[2] = pRGB[1];
   nacelleSignal[3] = pRGB[2];
   Serial.write(nacelleSignal, 4); 
}

void updateSaucerSectionDataRegister(){
   sectionSignal[0] = SERIAL_COMM_SAUCER_SECTION;
   sectionSignal[1] = (saucerSectionData & 0xFF);
   Serial.write(sectionSignal, 2);
}

void updateSaucerSection(byte section, byte set){
  if (set == 1){
     bitSet(saucerSectionData, (section-1)); 
  }  
  else {
     bitClear(saucerSectionData, (section-1)); 
  }
  updateSaucerSectionDataRegister();
}

void loop() {
  
   if ( testMode){
      runStartUpSequence();
      bRedAlertOn = true;
      testMode = false;
      return;
   }
   
   if (Serial.available() > 0) {

     // read the incoming byte:
     incomingByte = Serial.read();

     switch (incomingByte) {
       case SERIAL_COMM_POWER_OFF:
          Serial.write(SERIAL_COMM_POWER_OFF); 
          runShutdownSequence();
          break;
       case SERIAL_COMM_POWER_ON: //power on
          runStartUpSequence();
          Serial.write(SERIAL_COMM_POWER_ON);
          break;
       case SERIAL_COMM_TORPEDO:
          fireTorpedo();
          break;
       case SERIAL_COMM_PHASER_ON:
          Serial.write(SERIAL_COMM_PHASER_ON);
          break;
       case SERIAL_COMM_PHASER_OFF:
          Serial.write(SERIAL_COMM_PHASER_OFF);
          break;
       case SERIAL_COMM_START_WARP_DRIVE:
          Serial.write(SERIAL_COMM_START_WARP_DRIVE);
          break;
       case SERIAL_COMM_STOP_WARP_DRIVE:
          Serial.write(SERIAL_COMM_STOP_WARP_DRIVE);
          break;
       case SERIAL_COMM_INCREASE_WARP_DRIVE:
          Serial.write(SERIAL_COMM_INCREASE_WARP_DRIVE);
          break;
       case SERIAL_COMM_DECREASE_WARP_DRIVE:
          Serial.write(SERIAL_COMM_DECREASE_WARP_DRIVE);
          break;
       case SERIAL_COMM_IMPULSE_DRIVE:
          Serial.readBytes(&newImpulseLevel,1);
          setImpulseDrive(newImpulseLevel);
          break;
          
       case SERIAL_COMM_DEFLECTOR_COLOR:
          byte color[3];
          Serial.readBytes(&color[0],3);
          setDeflector(color);
          break;

       case SERIAL_COMM_NACELLE_COLOR:
          nacelleSignal[0]=SERIAL_COMM_NACELLE_COLOR;
          Serial.readBytes(&nacelleSignal[1],3);
          Serial.write(nacelleSignal, 4); 
          break;
          
       case SERIAL_COMM_CRYSTAL_COLOR:
          crystalSignal[0]=SERIAL_COMM_CRYSTAL_COLOR;
          Serial.readBytes(&crystalSignal[1],3);
          Serial.write(crystalSignal, 4); 
          break;
          
       case SERIAL_COMM_SAUCER_SECTION_1:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_1, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_2:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_2, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_3:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_3, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_4:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_4, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_5:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_5, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_6:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_6, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_BRIDGE:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_BRIDGE, stateIn);
          break;
       case SERIAL_COMM_SAUCER_SECTION_LOUNGE:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          updateSaucerSection(SAUCER_SECTION_LOUNGE, stateIn);
          break;
       case SERIAL_COMM_FLOOD_1:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_FLOOD_2, stateIn);
          break;
       case SERIAL_COMM_FLOOD_2: 
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_FLOOD_1, stateIn);
          break;
       case SERIAL_COMM_HANGER_SECTION:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_HANGER, stateIn);
          break;
       case SERIAL_COMM_AFT_SECTION:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_AFT_LIGHTS, stateIn);
          break;
       case SERIAL_COMM_BELLY_SECTION:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_DOWN_BELOW, stateIn);
          break;
       case SERIAL_COMM_ENGINEERING_SECTION_1:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_PORT_LIGHTS, stateIn);
          break;
       case SERIAL_COMM_ENGINEERING_SECTION_2:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_STARBOARD_LIGHTS, stateIn);
          break;
       case SERIAL_COMM_NECK_SECTION:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_NECK_LIGHTING, stateIn);
          break;
       case SERIAL_COMM_ARBORITUM:
          stateIn = 0;
          Serial.readBytes(&stateIn, 1);
          digitalWrite(PIN_ARBORITUM, stateIn);
          break;        
       case SERIAL_COMM_RED_ALERT_ON:
          //bRedLedOn = true;
          //digitalWrite(PIN_RED_ALERT, bRedLedOn);
          //digitalWrite(PIN_HANGER, false);
          bRedAlertOn = true;
          break;
       case SERIAL_COMM_RED_ALERT_OFF: 
          //bRedLedOn = false;
          //digitalWrite(PIN_RED_ALERT, bRedLedOn);
          //digitalWrite(PIN_HANGER, true);
          bRedAlertOn = false;
          break;
       default:
          break;
     } ///end switch
   } //if serial
}  //loop
