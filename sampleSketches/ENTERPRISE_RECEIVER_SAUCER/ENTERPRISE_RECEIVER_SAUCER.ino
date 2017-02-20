#include <EN1701-REFIT.h>
#include <SERIAL_COMM.h>

int incomingByte = 0;   // for incoming serial data

unsigned int sectionData = 0;
byte sectionSignalIn = 0;

byte newCrystalRGB[] = {25,25,25};
byte oldCrystalRGB[] = {0,0,0};

//pin assignments
#define PIN_NAVIGATION_BEACON 17
#define PIN_NAVIGATION_FLASHER 18
#define PIN_SR_SECTION_DATA 10
#define PIN_SR_ENABLE 11
#define PIN_SR_LATCH 12
#define PIN_SR_CLOCK 13

#define PIN_CRYSTAL_R 6
#define PIN_CRYSTAL_G 5
#define PIN_CRYSTAL_B 3
#define PIN_PHASER 14
#define PIN_IMPULSE 9

boolean bPowerOn = false;

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

  setCrystal();
}

void updateSectionDataRegister() {  
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (sectionData & 0xFF));
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

void setCrystal() {

    bool oper_R = (oldCrystalRGB[0] >= newCrystalRGB[0]) ? 0:1;
    bool oper_G = (oldCrystalRGB[1] >= newCrystalRGB[1]) ? 0:1;
    bool oper_B = (oldCrystalRGB[2] >= newCrystalRGB[2]) ? 0:1;
      
    for (int analogCount=0; analogCount<=255; analogCount++){
        
        if ( oldCrystalRGB[0] != newCrystalRGB[0] ) {
           if ( oper_R ) {
             analogWrite(PIN_CRYSTAL_R, (255-(oldCrystalRGB[0]++)));
           }
           else {
             analogWrite(PIN_CRYSTAL_R, (255-(oldCrystalRGB[0]--)));
           }
        }

        if ( oldCrystalRGB[1] != newCrystalRGB[1] ) {
           if ( oper_G ) {
             analogWrite(PIN_CRYSTAL_G, (255-(oldCrystalRGB[1]++)));
           }
           else {
             analogWrite(PIN_CRYSTAL_G, (255-(oldCrystalRGB[1]--)));
           }
        }

        if ( oldCrystalRGB[2] != newCrystalRGB[2] ) {
           if ( oper_B ) {
             analogWrite(PIN_CRYSTAL_B, (255-(oldCrystalRGB[2]++)));
           }
           else {
             analogWrite(PIN_CRYSTAL_B, (255-(oldCrystalRGB[2]--)));
           }
        }
        delay(7);
    }  
}

/*void powerSaucerSectionUp(){
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
}*/

void runShutdownSequence(){
  bPowerOn = false;

  digitalWrite(PIN_NAVIGATION_BEACON, LOW); 
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);   
  digitalWrite(PIN_PHASER, LOW);
  //powerSaucerSectionDown();
}

void runStartUpSequence() {

  //powerSaucerSectionUp();

  //start nav lights
  bPowerOn = true;
  //bNavBeaconOn=true;
  //bNavFlasherOn=true;
  //start dome
  //start impulse engines
  //start warp engines 
  //fade in deflector
  //delay(2000); 

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
       case SERIAL_COMM_CRYSTAL_COLOR:
          Serial.readBytes(newCrystalRGB, 3);
          setCrystal();       
          break;
       case SERIAL_COMM_SAUCER_SECTION:
          Serial.readBytes(&sectionSignalIn,1);
          for (int set=0; set<8; set++){
             if (bitRead(sectionSignalIn,set)==1){
                bitSet(sectionData, set);
             }
             else {
                bitClear(sectionData, set);
             }
          }
          updateSectionDataRegister();
          break;
       default:
          /*switch (int(incomingByte/10) * 10){
            case SERIAL_COMM_IMPULSE_DRIVE:
               break;
            case SERIAL_COMM_WARP_DRIVE:
               break;
          }*/
          break;
     }
   }
}
