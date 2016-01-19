 #include <IRLib.h>
  
class IRReader {
  public:
  IRrecv My_Receiver;
  IRdecodeNEC My_Decoder;
  
  long lastDecodedValue = 0;
  int latchDelay = 500;
  int brightness = 0;
  
  public:
  IRReader():My_Receiver(11) {
     My_Receiver.enableIRIn(); // Start the receiver
     Serial.begin(9600);
  }

  int getLatchDelay(){
    return latchDelay;
  }

  int getBrightness() {
    return brightness;
  }
  
  void readIR_latchDelay () {
    if (My_Receiver.GetResults(&My_Decoder))  {
       My_Decoder.decode();    //Decode the data

       if ( My_Decoder.value == 0xffffffff ) {
          My_Decoder.value = lastDecodedValue;
       }
       Serial.println(My_Decoder.value);
       
       switch (My_Decoder.value){
         case 0xff12ed: //down
            Serial.println("DIMMER");
            if (brightness >= 5 ) {
              brightness -= 5;
            }
            lastDecodedValue = My_Decoder.value;
            break;
          case 0xffa25d: //up
            Serial.println("BRIGHTER");
            if (brightness <= 250){
              brightness += 5;
            }
            lastDecodedValue = My_Decoder.value;
            break;
          case 0xff22dd: //left
            Serial.println("FASTER");
            if (latchDelay >= 10){
              latchDelay -= 10;
            }
            lastDecodedValue = My_Decoder.value;
            break;
          case 0xffe01f: //right
            Serial.println("SLOWER");
            if (latchDelay <= 1000){
              latchDelay += 10;
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
}; //end IRReader

//-------
int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

//int potPin = 0;
int brightnessPin = 9;

 
byte leds = 0;
byte bitToSet = 1;
byte direction = 1;

//----------
unsigned long previousMillis = 0;
unsigned long interval = 500; 

IRReader* mIRReader;

void setup()
{
  mIRReader = new IRReader;
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
}
 
void loop() {
    
  unsigned long currentMillis = millis();
  mIRReader->readIR_latchDelay();
  
  if (currentMillis - previousMillis >= mIRReader->getLatchDelay()) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      analogWrite(9, mIRReader->getBrightness());

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



