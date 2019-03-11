#include <IRLib.h>
 
//Create a receiver object to listen on pin 11
IRrecv My_Receiver(11);
 
//Create a decoder object
IRdecodeNEC My_Decoder;
long lastDecodedValue = 0;

//-------
int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

int potPin = 0;
int latchDelay = 500;
 
byte leds = 0;
byte bitToSet = 1;
byte direction = 1;

//----------
unsigned long previousMillis = 0;
long interval = 500; 

void setup()
{
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  Serial.begin(9600);
  
  My_Receiver.enableIRIn(); // Start the receiver
}
 
void loop() {

  readIR_latchDelay();
    
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= latchDelay) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      leds = 0;
      bitSet(leds, bitToSet);
      updateShiftRegister();

      if ( bitToSet >= 7 || bitToSet <= 0) {
        direction = direction * -1;
      }

      bitToSet += direction;
  }
    
  //potValue = analogRead(potPin);
}

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

int readIR_latchDelay () {

    if (My_Receiver.GetResults(&My_Decoder))  {
       
       My_Decoder.decode();    //Decode the data

       if ( My_Decoder.value == 0xffffffff ) {
          My_Decoder.value = lastDecodedValue;
       }
       
       switch (My_Decoder.value){
         case 0xffa25d: //down
            Serial.println("SLOWER");
            if (latchDelay <= 1000) {
              latchDelay += 10;
            }
            lastDecodedValue = My_Decoder.value;
            break;
          case 0xff12ed: //up
            Serial.println("FASTER");
            if (latchDelay >= 10){
              latchDelay -= 10;
            }
            lastDecodedValue = My_Decoder.value;
            break;
         default:
            lastDecodedValue = 0;
            break;         
       }
       My_Receiver.resume(); //Restart the receiver
   }
   
}

