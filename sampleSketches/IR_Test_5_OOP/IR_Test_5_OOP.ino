 #include <IRLib.h>
  
class IRReader {
  public:

  long lastDecodedValue = 0;
  int latchDelay = 500;
  int brightness = 0;
  
  IRrecv My_Receiver;
  IRdecodeNEC My_Decoder;
  
  public:

  IRReader(int rp):My_Receiver(rp) {
     My_Receiver.enableIRIn(); // Start the receiver
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
            Serial.println("BRIGHTER");
            if (brightness >= 5 ) {
              brightness -= 5;
            }
            lastDecodedValue = My_Decoder.value;
            break;
          case 0xffa25d: //up
            Serial.println("DIMMER");
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
#define LATCHPIN 5
#define CLOCKPIN 6
#define DATAPIN 4
#define PWMBRIGHTNESSPIN 9
#define RECEIVER_PIN 11

byte leds = 0;
byte bitToSet = 1;
byte direction = 1;

//----------
unsigned long previousMillis = 0;
IRReader *mIRReader;

void setup()
{
  Serial.begin(9600);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(DATAPIN, OUTPUT);  
  pinMode(CLOCKPIN, OUTPUT);
  mIRReader = new IRReader(RECEIVER_PIN);

}
 
void loop() {

  unsigned long currentMillis = millis();
  mIRReader->readIR_latchDelay();
  
  if (currentMillis - previousMillis >= mIRReader->getLatchDelay()) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      analogWrite(PWMBRIGHTNESSPIN, mIRReader->getBrightness());

      leds = 0;
      bitSet(leds, bitToSet);
      updateShiftRegister();

      if ( bitToSet >= 7 || bitToSet <= 0) {
        direction = direction * -1;
      }

      bitToSet += direction;
  }
}

void updateShiftRegister()
{
   digitalWrite(LATCHPIN, LOW);
   shiftOut(DATAPIN, CLOCKPIN, LSBFIRST, leds);
   digitalWrite(LATCHPIN, HIGH);
}



