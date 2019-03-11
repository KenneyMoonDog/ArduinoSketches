#include <IRLib.h>
 
//Create a receiver object to listen on pin 11
IRrecv My_Receiver(11);
 
//Create a decoder object
IRdecodeNEC My_Decoder;
int increment = 0;
int lastDecodedValue = 0;

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

int potPin = 0;
int potValue = 500;
 
byte leds = 0;
byte bitToSet = 0;
int direction = 1;

 
void setup()
{
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  Serial.begin(9600);
  
  My_Receiver.enableIRIn(); // Start the receiver
}
 
void loop() {
    
  for (int i = 0; i < 8; i++)
  {
    //potValue = analogRead(potPin);
    readAndDecodeIR();
    
    leds = 0;
    if ( direction == 1 ) {
      bitToSet = i;
    }
    else {
      bitToSet = 7 - i;
    }

    if ( direction == 1 && bitToSet != 7 || direction == -1 && bitToSet != 0 ){
         bitSet(leds, bitToSet);
         updateShiftRegister();
         delay(potValue);
    }
  }

  direction = direction * -1;
}

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

void readAndDecodeIR () {
    if (My_Receiver.GetResults(&My_Decoder))  {
       
       My_Decoder.decode();    //Decode the data
       increment = 0;
       
       switch (My_Decoder.value){
         case 0xff12ed: //up
            if (potValue <= 1000) {
               increment = 100;
            }
            lastDecodedValue = My_Decoder.value;
            break;
         case 0xffa25d: //down
            if (potValue >= 100){
               increment = -100;
            }
            lastDecodedValue = My_Decoder.value;
            break;
         case 0xffffff: //repeat
            switch ( lastDecodedValue ) {
              case 0xffa25d:
                increment = -100;
                break;
              case 0xff12ed:
                increment = 100;
                break; 
            }
            break; 
         default:
            increment = 0;
            break;         
       }
       potValue += increment;
   }
      My_Receiver.resume(); //Restart the receiver
}

