
#include <WiFi.h>
#include "types.h"
#include "TinyGPS++.h";
#include "HardwareSerial.h";
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

//#include "SSD1306.h"


// Replace with your network credentials (STATION)
const char* ssid     = "JeffreysHotspot";
const char* password = "JeffreyRocks01";


unsigned long previousMillis = 0;
unsigned long interval = 30000;

void initWiFi() {
  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  updateWifiStatusLeds();
}

//---------------------------------------
//GPS Globals
//GpsDataState_t gpsState = {};

TinyGPSPlus gps;
HardwareSerial SerialGPS(2);
uint32_t nextSerialTaskTs = 0;
#define GPS_TASK_SERIAL_RATE 5000;

double dLatitude = 0;
double dLongitude = 0;
double dAltitude = 0;
int mNumSat = 0;

//--------------PIN ASSIGNMENTS-------------------
#define GPS_PIN_RX 16
#define GPS_PIN_TX 17

#define BUTTON_PIN_1 15
#define BUTTON_PIN_2 0
#define BUTTON_PIN_3 4
#define BUTTON_PIN_4 5
#define BUTTON_PIN_5 21
#define BUTTON_PIN_INTERRUPT 34

//#define PIN_SR_ENABLE 11
//#define PIN_SR_LATCH 12
//#define PIN_SR_CLOCK 13

#define LED_PIN   13
#define LED_COUNT 5
#define LED_PANIC_STATUS 0
#define LED_WIFI_STATUS 1
#define LED_GPS_STATUS 2
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/*const long red = 65535;
const long pink = 65536*5/6;
const long blue = 65536*2/3;
const long teal = 65536/2;
const long green = 65536/3;
const long yellow = 65536/6;*/



enum GPS_STATUS {
  GPS_DISCONNECTED,
  GPS_CONNECTED_NO_DATA,
  GPS_CONNECTED
};

enum WIFI_STATUS {
   WIFI_DISCONNECTED,
   WIFI_CONNECTED 
};

typedef struct{
     GPS_STATUS gps_status = GPS_DISCONNECTED;
     WIFI_STATUS wifi_status = WIFI_DISCONNECTED;
} TRACKER_STATUS;

TRACKER_STATUS tracker_status;

class jButton {
  public:
    jButton(uint8_t pin) {
      PIN=pin;
    }

    virtual volatile bool readButtonState(){
      bool is_pressed = pressed;
      if (resetOnRead) {
        pressed = false;
      }

      return is_pressed;
    }

    virtual volatile void setResetOnRead(bool set) {
      resetOnRead = set;
    }

    virtual volatile void onButtonChangeDown(){
      if ( digitalRead(PIN) == false ) {
        if ((millis() - debounceTarget) > buttonDebounceDelay) { //then this should be a state change
          debounceTarget = millis();
          //Serial.println("BUTTON PRESS TRUE");
          pressed = true;
        }
      }
    }

    virtual volatile uint8_t getPIN() {
      return PIN;
    }
  
  private:
    volatile uint8_t PIN = 0;
    volatile unsigned long buttonDebounceDelay = 250;
    volatile unsigned long debounceTarget = 0;
    volatile bool pressed = false;
    volatile bool resetOnRead = true;
}; //class jButton

class jLED {
  public:
  jLED(uint8_t pin) {
    PIN = pin; 
  }

  public:
    uint8_t PIN = 0;
    uint8_t rValue= 0;
    uint8_t gValue= 0;
    uint8_t bValue= 0; 
    bool flash = false; 
};   //class jLED

jButton button_1(BUTTON_PIN_1);
jButton button_2(BUTTON_PIN_2);
jButton button_3(BUTTON_PIN_3);
jButton button_4(BUTTON_PIN_4);
jButton button_5(BUTTON_PIN_5);

void IRAM_ATTR buttonPress() {
  //Serial.println("Interrupt FIRED");
  button_1.onButtonChangeDown();
  button_2.onButtonChangeDown();
  button_3.onButtonChangeDown();
  button_4.onButtonChangeDown();
  button_5.onButtonChangeDown();
}


//---------------------------------------

void setup() {

  Serial.begin(115200); //Serial port of USB
  delay(10);
    /*Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());*/
  
  pinMode(BUTTON_PIN_INTERRUPT, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN_INTERRUPT, buttonPress, FALLING);

  pinMode(button_1.getPIN(), INPUT_PULLUP);
  pinMode(button_2.getPIN(), INPUT_PULLUP);
  pinMode(button_3.getPIN(), INPUT_PULLUP);
  pinMode(button_4.getPIN(), INPUT_PULLUP);
  pinMode(button_5.getPIN(), INPUT_PULLUP);
  
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  initializeLEDs();

  initWiFi();   
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void initializeLEDs(){

    for (int nPix = 0; nPix < LED_COUNT; nPix++){
      strip.setPixelColor(LED_GPS_STATUS, 200, 0, 0);
    } //end for
    
    strip.show(); // Update strip with new contents
}


void checkWifiStatus() {
    unsigned long currentMillis = millis();

    if (WiFi.status() != WL_CONNECTED) {
      if (currentMillis - previousMillis >=interval) {
   
        //Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        previousMillis = currentMillis;
        tracker_status.wifi_status = WIFI_DISCONNECTED;
      }
   }
   else {
     tracker_status.wifi_status = WIFI_CONNECTED;
   }

   updateWifiStatusLeds();
}

void updateWifiStatusLeds() {
  switch (tracker_status.wifi_status) {
     case WIFI_DISCONNECTED:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(red, 255, 120)));
       strip.setPixelColor(LED_WIFI_STATUS, 200, 0, 0);

       break;
     case WIFI_CONNECTED:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(green, 255, 120)));
       strip.setPixelColor(LED_WIFI_STATUS, 0, 200, 0);
       break;
     default:
       break;
  }

  strip.show();
}

void checkGPSStatus() {

   bool gpsDataReady = false;
    
   while (SerialGPS.available() > 0){
     gps.encode(SerialGPS.read());
     gpsDataReady = true;
   }
    
   nextSerialTaskTs = millis() + GPS_TASK_SERIAL_RATE;

   if (gpsDataReady) {
     dLatitude = gps.location.lat();
     dLongitude = gps.location.lng();
     dAltitude = gps.altitude.meters();
     mNumSat = gps.satellites.value();

     if ((dLatitude == 0) || dLongitude == 0) {
       tracker_status.gps_status = GPS_CONNECTED_NO_DATA;
     }
     else {
       tracker_status.gps_status = GPS_CONNECTED;
     }
   } 
   else {
    // tracker_status.gps_status = GPS_DISCONNECTED;
   }

   updateGPSStatusLeds();
}

void updateGPSStatusLeds() {
  switch (tracker_status.gps_status) {
     case GPS_DISCONNECTED:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(red, 255, 120)));
       strip.setPixelColor(LED_GPS_STATUS, 200, 0, 0);

       break;
     case GPS_CONNECTED_NO_DATA:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(yellow, 255, 120)));
       strip.setPixelColor(LED_GPS_STATUS, 200,55,0);
       break;
     case GPS_CONNECTED:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(green, 255, 120)));
       strip.setPixelColor(LED_GPS_STATUS, 0, 200, 0);
       break;
     default:
       break;
  }

  strip.show();
}

void loop() {

   checkWifiStatus();
   checkGPSStatus();
 
   if (button_1.readButtonState()) {
     Serial.print("LAT="); Serial.println(dLatitude, 6);
     Serial.print("LONG="); Serial.println(dLongitude, 6);
     Serial.print("ALT="); Serial.println(dAltitude, 2);
     Serial.print("Sats="); Serial.println(mNumSat);
   }
   else if (button_2.readButtonState()) {
     Serial.println("button 2 pressed");
   }
   else if (button_3.readButtonState()) {
     Serial.println("button 3 pressed");
   }
   else if (button_4.readButtonState()) {
     Serial.println("button 4 pressed");
   }
   else if (button_5.readButtonState()) {
     Serial.println("button 5 pressed");
   }

   //updateStatusLeds();
}
