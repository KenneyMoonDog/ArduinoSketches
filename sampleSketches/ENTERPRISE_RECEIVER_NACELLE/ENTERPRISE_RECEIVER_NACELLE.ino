#include <SERIAL_COMM.h>

volatile unsigned long previousMillis = 0;
volatile unsigned long coil_1_start_time = 0;
volatile unsigned long coil_2_start_time = 0;
volatile unsigned long coil_3_start_time = 0;

volatile unsigned long coil_1_end_time = 0;
volatile unsigned long coil_2_end_time = 0;
volatile unsigned long coil_3_end_time = 0;

volatile boolean coil_1_on = false;
volatile boolean coil_2_on = false;
volatile boolean coil_3_on = false;

int incomingByte = 0;   // for incoming serial data
byte newNacelleRGB[] = {10,10,10};
byte oldNacelleRGB[] = {0,0,0};

byte newWarpPowerLevel = 25;
byte oldWarpPowerLevel = 25;
int coilPeriod = 250;
boolean bWarpOn = false;

#define PIN_NAVIGATION_FLASHER 18
#define PIN_NACELLE_R 6
#define PIN_NACELLE_G 5
#define PIN_NACELLE_B 3

#define PIN_COIL_1 9
#define PIN_COIL_2 10
#define PIN_COIL_3 11

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 7 //ms
boolean bPowerOn = false;

void setup() {    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_NACELLE_R, OUTPUT);
  pinMode(PIN_NACELLE_G, OUTPUT);
  pinMode(PIN_NACELLE_B, OUTPUT);

  pinMode(PIN_COIL_1, OUTPUT);
  pinMode(PIN_COIL_2, OUTPUT);
  pinMode(PIN_COIL_3, OUTPUT);
  
  analogWrite(PIN_NACELLE_R, 255);
  analogWrite(PIN_NACELLE_G, 255);
  analogWrite(PIN_NACELLE_B, 255);
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);

  resetWarpEngine();
  
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {
    previousMillis = currentMillis;
    checkNacelleLevel();
    checkWarpEngine();
  }
} 

void resetWarpEngine() {
  unsigned long startTime = millis();
  coil_1_start_time = startTime + coilPeriod;
  coil_1_end_time = startTime + (2*coilPeriod);
  coil_2_start_time = startTime + (2*coilPeriod);
  coil_2_end_time = startTime + (3*coilPeriod);
  coil_3_start_time = startTime + (3*coilPeriod);
  coil_3_end_time = startTime + (4*coilPeriod);
}

void setCoilPeriod(byte powerLevel) {
   coilPeriod = (-4*powerLevel) + 500;
}

void checkWarpPower() {
  bool operPower = (oldWarpPowerLevel >= newWarpPowerLevel) ? 1:0;

  if ( oldWarpPowerLevel != newWarpPowerLevel ) {
     if ( operPower ) {
       oldWarpPowerLevel--;
     }
     else {
       oldWarpPowerLevel++;
     }
     setCoilPeriod(oldWarpPowerLevel);
   }
}

void checkNacelleLevel() {

    bool oper_R = (oldNacelleRGB[0] >= newNacelleRGB[0]) ? 0:1;
    bool oper_G = (oldNacelleRGB[1] >= newNacelleRGB[1]) ? 0:1;
    bool oper_B = (oldNacelleRGB[2] >= newNacelleRGB[2]) ? 0:1;
  
     if ( oldNacelleRGB[0] != newNacelleRGB[0] ) {
        if ( oper_R ) {
          analogWrite(PIN_NACELLE_R, (255-(++oldNacelleRGB[0])));
        }
        else {
          analogWrite(PIN_NACELLE_R, (255-(--oldNacelleRGB[0])));
        }
     }

     if ( oldNacelleRGB[1] != newNacelleRGB[1] ) {
        if ( oper_G ) {
          analogWrite(PIN_NACELLE_G, (255-(++oldNacelleRGB[1])));
        }
        else {
          analogWrite(PIN_NACELLE_G, (255-(--oldNacelleRGB[1])));
        }
     }

     if ( oldNacelleRGB[2] != newNacelleRGB[2] ) {
        if ( oper_B ) {
          analogWrite(PIN_NACELLE_B, (255-(++oldNacelleRGB[2])));
        }
        else {
          analogWrite(PIN_NACELLE_B, (255-(--oldNacelleRGB[2])));
        }
     }
}

void checkWarpEngine(){
  
  unsigned long currentMillis = millis();

  if (coil_1_on) {
    if ( currentMillis > coil_1_end_time ) { 
       coil_1_start_time = currentMillis + (2*coilPeriod);
       coil_1_on = false;
       digitalWrite(PIN_COIL_1, LOW);
    }
  }
  else {
    if ( currentMillis > coil_1_start_time ) { 
       coil_1_end_time = currentMillis + coilPeriod;
       coil_1_on = true;
       if (bWarpOn){
         digitalWrite(PIN_COIL_1, HIGH);
       }
    }
  }

  if (coil_2_on) {
    if ( currentMillis > coil_2_end_time ) { 
       coil_2_start_time = currentMillis + (2*coilPeriod);
       coil_2_on = false;
       digitalWrite(PIN_COIL_2, LOW);
    }
  }
  else {
    if ( currentMillis > coil_2_start_time ) { 
       coil_2_end_time = currentMillis + coilPeriod;
       coil_2_on = true;
       if (bWarpOn){
         digitalWrite(PIN_COIL_2, HIGH);
       }
    }
  }

  if (coil_3_on) {
    if ( currentMillis > coil_3_end_time ) { 
       coil_3_start_time = currentMillis + (2*coilPeriod);
       coil_3_on = false;
       digitalWrite(PIN_COIL_3, LOW);
    }
  }
  else {
    if ( currentMillis > coil_3_start_time ) { 
       coil_3_end_time = currentMillis + coilPeriod;
       coil_3_on = true;
       if (bWarpOn){
         digitalWrite(PIN_COIL_3, HIGH);
       }
    }
  }
}

void runShutdownSequence(){
  bPowerOn = false;
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);   
}

void runStartUpSequence() {
  //start nav lights
  bPowerOn = true;
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
       case SERIAL_COMM_NAV_FLASHER_ON:
          if (bPowerOn) {
            digitalWrite(PIN_NAVIGATION_FLASHER, HIGH);
          }
          break;
       case SERIAL_COMM_NAV_FLASHER_OFF:
          digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
          break;
       case SERIAL_COMM_NACELLE_COLOR:
          Serial.readBytes(newNacelleRGB, 3);     
          break;
       case SERIAL_COMM_INCREASE_WARP_DRIVE:
          coilPeriod+=25;
          resetWarpEngine();
          break;
       case SERIAL_COMM_DECREASE_WARP_DRIVE:
          coilPeriod-=25;
          resetWarpEngine();
          break;       
       case SERIAL_COMM_START_WARP_DRIVE:
          bWarpOn = true;
          break;
       case SERIAL_COMM_STOP_WARP_DRIVE:
          bWarpOn = false;
          break;     
       default:
          break;
     }
   }
}
