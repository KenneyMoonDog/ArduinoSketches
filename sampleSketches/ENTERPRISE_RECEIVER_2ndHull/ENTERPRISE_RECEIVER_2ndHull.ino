#include <EN1701-REFIT.h>
#include <SERIAL_COMM.h>

int incomingByte = 0;   // for incoming serial data
unsigned long saucerSectionData = 0;
unsigned long hullSectionData = 0;

#define PIN_DEFLECTOR_R 6
#define PIN_DEFLECTOR_G 5
#define PIN_DEFLECTOR_B 3
#define PIN_PHOTON_TORPEDO_1 9 
#define PIN_PHOTON_TORPEDO_2 10

#define PIN_SR_SECTION_DATA 13
#define PIN_SR_ENABLE 14
#define PIN_SR_LATCH 12
#define PIN_SR_CLOCK 11

#define PIN_NAVIGATION_FLASHER 8

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 100 //ms
#define FAST_RECEIVER_INTERRUPT_FREQUENCY 10 //ms
#define flashTimeOn 10
#define flashTimeOff 11
#define beaconTimeOn 25
#define beaconTimeOff 35

byte sectionSignal[] = {0,0};
volatile unsigned long slowPreviousMillis = 0;
volatile unsigned long fastPreviousMillis = 0;
boolean bPowerOn = false;
boolean bNavBeaconOn = false;
boolean bNavFlasherOn = false;

byte crystalSignal[] = {0,0,0,0};
byte newDeflectorRGB[] = {0,0,0};
byte oldDeflectorRGB[] = {0,0,0};
byte impulseLevelSignal[] = {SERIAL_COMM_IMPULSE_DRIVE, 0};

byte torpedo_tube = PIN_PHOTON_TORPEDO_1;

void setup() {
  pinMode(PIN_SR_ENABLE, OUTPUT);
  digitalWrite(PIN_SR_ENABLE,HIGH);
    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_SR_CLOCK, OUTPUT);  
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_SR_SECTION_DATA, OUTPUT);
  pinMode(PIN_DEFLECTOR_R, OUTPUT);
  pinMode(PIN_DEFLECTOR_G, OUTPUT);
  pinMode(PIN_DEFLECTOR_B, OUTPUT);
  pinMode(PIN_PHOTON_TORPEDO_1, OUTPUT);
  pinMode(PIN_PHOTON_TORPEDO_2, OUTPUT);

  setDeflector(colorOff);
  analogWrite(PIN_DEFLECTOR_R,255);
  analogWrite(PIN_DEFLECTOR_G,255);
  analogWrite(PIN_DEFLECTOR_B,255);
 
  updateSaucerSectionDataRegister();
  updateHullSectionDataRegister();
  
  digitalWrite(PIN_SR_ENABLE,LOW);
 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - slowPreviousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {  //execute any timed operations every INTERRRUPT FREQ ms
    // save the last time you did a repeatable item clear
    slowPreviousMillis = currentMillis;
    updateNavBeacon(bNavBeaconOn);
    randomSaucerSectionUpdate(bPowerOn);
  } //end if timer

  if (currentMillis - fastPreviousMillis >= FAST_RECEIVER_INTERRUPT_FREQUENCY) {
    fastPreviousMillis = currentMillis;
    checkDeflectorLevel();
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

void updateNavBeacon(boolean bPowerOn){

  static byte bCounter=0;
  static boolean bBeaconOn=false;
  static boolean bFlasherOn=false;
  
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
    }
  }
}

void randomSaucerSectionUpdate(boolean bPowerOn) { //called every POLLING_FREQUENCY ms
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
}

void updateSaucerSectionDataRegister(){
   sectionSignal[0] = SERIAL_COMM_SAUCER_SECTION;
   sectionSignal[1] = (saucerSectionData & 0xFF);
   Serial.write(sectionSignal, 2); 
}

void setImpulseDrive(byte level) {
   impulseLevelSignal[1] = level;
   Serial.write(impulseLevelSignal, 2);
}

void updateHullSectionDataRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (hullSectionData & 0xFF));
   //shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF00) >> 8);
   digitalWrite(PIN_SR_LATCH, HIGH);
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

void powerSaucerSectionUp(){
  for (int section=0; section<8; section++){
    bitSet(saucerSectionData, section);
    updateSaucerSectionDataRegister();
    delay(750);
  }
}

void powerSaucerSectionDown(){

  for (int section=0; section<8; section++){
    bitClear(saucerSectionData, section);
    updateSaucerSectionDataRegister();
    delay(750);
  }
}

void runShutdownSequence(){
  bNavBeaconOn=false;
  bNavFlasherOn=false;
  bPowerOn = false;
  setImpulseDrive(0);
  powerSaucerSectionDown();
 
  setDeflector(colorOff);
  setCrystal(colorWhite);
}

void runStartUpSequence() {
  
  setCrystal(colorAmber);
  delay(2000);
  powerSaucerSectionUp();
 
  //start nav lights
  bPowerOn = true;
  bNavBeaconOn=true;
  bNavFlasherOn=true;

  setDeflector(colorAmber);
  setImpulseDrive(5);
}

void setCrystal(byte pRGB[]) {
   crystalSignal[0] = SERIAL_COMM_CRYSTAL_COLOR;
   crystalSignal[1] = pRGB[0];
   crystalSignal[2] = pRGB[1];
   crystalSignal[3] = pRGB[2];
   Serial.write(crystalSignal, 4); 
}

void buttonPressAction(byte button){
    int sectionNumber = button - 1;
    if (bPowerOn) {
       if (bitRead(saucerSectionData, sectionNumber) == 1){
          bitClear(saucerSectionData, sectionNumber);
       }
       else {
          bitSet(saucerSectionData, sectionNumber);
       }
       updateSaucerSectionDataRegister();
    }  
}

void loop() {

   if (Serial.available() > 0) {

     // read the incoming byte:
     incomingByte = Serial.read();

     switch (incomingByte) {
       case SERIAL_COMM_POWER_OFF: 
          runShutdownSequence();
          Serial.write(SERIAL_COMM_POWER_OFF);
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
       case SERIAL_COMM_WARP_DRIVE:
          setCrystal(colorBlue);
          setDeflector(colorBlue);
          setImpulseDrive(5);
          break;
       case SERIAL_COMM_IMPULSE_DRIVE:
          setCrystal(colorAmber);
          setDeflector(colorAmber);
          setImpulseDrive(255);
          break;
       case SERIAL_COMM_BUTTON_1:
          buttonPressAction(1);
          break;
       case SERIAL_COMM_BUTTON_2:
          buttonPressAction(2);
          break;
       case SERIAL_COMM_BUTTON_3:
          buttonPressAction(3);
          break;
       case SERIAL_COMM_BUTTON_4:
          buttonPressAction(4);
          break;
       case SERIAL_COMM_BUTTON_5:
          buttonPressAction(5);
          break;
       case SERIAL_COMM_BUTTON_6:
          buttonPressAction(6);
          break;
       case SERIAL_COMM_BUTTON_7:
          buttonPressAction(7);
          break;
       case SERIAL_COMM_BUTTON_8:
          buttonPressAction(8);
          break;
     } ///end switch
   } //if serial
}  //loop
