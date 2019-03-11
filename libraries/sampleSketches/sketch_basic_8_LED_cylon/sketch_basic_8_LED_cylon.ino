/*
A little sketch to simulate CYLON eye scanner thing
*/
 
int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

int potPin = 0;
int potValue = 0;
 
byte leds = 0;
byte bitToSet = 0;
int direction = 1;
 
void setup() 
{ 
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
}
 
void loop() 
{
  for (int i = 0; i < 8; i++)
  {
    potValue = analogRead(potPin);
    
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
