 #include <IRLib.h>

#define SECTION_0 0
#define SECTION_1 1
#define SECTION_2 2

class StateReader {
  public:

  long lastDecodedValue = 0;
  int latchDelay = 500;
  int brightness = 0;

  unsigned int *pCurrentState; 
  unsigned int *pOldState;
  
  IRrecv mReceiver;
  IRdecodeNEC mDecoder;
  
  public:

  StateReader(int rp):mReceiver(rp) {
     mReceiver.enableIRIn(); // Start the receiver
     pCurrentState = new unsigned int(0);
     pOldState = new unsigned int(0);

     bitSet(*pCurrentState, SECTION_2);
  }

  int getLatchDelay(){
    return latchDelay;
  }

  int getBrightness() {
    return brightness;
  }
  
  void readIRCommand() {

    if (mReceiver.GetResults(&mDecoder))  {
       mDecoder.decode();    //Decode the data

       if ( mDecoder.value == 0xffffffff ) {
          switch (lastDecodedValue) {
            case 0xffd22d:
              lastDecodedValue = 0;
              break;
          }
          mDecoder.value = lastDecodedValue;
       }
       Serial.println(mDecoder.value);
       Serial.println(*pCurrentState);
       
       switch (mDecoder.value){
         case 0xffd22d: //power
            Serial.println("POWER");
            break;
         case 0xff12ed: //down
            Serial.println("BRIGHTER");
            if (brightness >= 5 ) {
              brightness -= 5;
            }
            break;
          case 0xffa25d: //up
            Serial.println("DIMMER");
            if (brightness <= 250){
              brightness += 5;
            }
            break;
          case 0xff22dd: //left
            Serial.println("FASTER");
            if (latchDelay >= 10){
              latchDelay -= 10;
            }
            break;
          case 0xffe01f: //right
            Serial.println("SLOWER");
            if (latchDelay <= 1000){
              latchDelay += 10;
            }
            break;
         default:
            mDecoder.value = 0;
            break;         
       }
       lastDecodedValue = mDecoder.value;
       mReceiver.resume(); //Restart the receiver
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
StateReader *pStateReader;

void setup()
{
  Serial.begin(9600);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(DATAPIN, OUTPUT);  
  pinMode(CLOCKPIN, OUTPUT);
  pStateReader = new StateReader(RECEIVER_PIN);
}
 
void loop() {

  unsigned long currentMillis = millis();
  pStateReader->readIRCommand();
  
  if (currentMillis - previousMillis >= pStateReader->getLatchDelay()) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      analogWrite(PWMBRIGHTNESSPIN, pStateReader->getBrightness());

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



