#include <EN1701-REFIT.h>
#include <SERIAL_COMM.h>

volatile unsigned long previousMillis = 0;

int incomingByte = 0;   // for incoming serial data

byte newNacelleRGB[] = {10,10,10};
byte oldNacelleRGB[] = {0,0,0};

#define PIN_NAVIGATION_FLASHER 18
#define PIN_NACELLE_R 6
#define PIN_NACELLE_G 5
#define PIN_NACELLE_B 3

//timer constants
#define RECEIVER_INTERRUPT_FREQUENCY 7 //ms

boolean bPowerOn = false;

void setup() {    
  Serial.begin(9600);
  pinMode(PIN_NAVIGATION_FLASHER, OUTPUT);   
  pinMode(PIN_NACELLE_R, OUTPUT);
  pinMode(PIN_NACELLE_G, OUTPUT);
  pinMode(PIN_NACELLE_B, OUTPUT);
  
  analogWrite(PIN_NACELLE_R, 255);
  analogWrite(PIN_NACELLE_G, 255);
  analogWrite(PIN_NACELLE_B, 255);
  digitalWrite(PIN_NAVIGATION_FLASHER, LOW);
  
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= RECEIVER_INTERRUPT_FREQUENCY) {
    previousMillis = currentMillis;
    checkNacelleLevel();
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
       default:
          break;
     }
   }
}
