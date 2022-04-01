
#include <WiFi.h>
//#include "types.h";
#include "TinyGPS++.h";
#include "HardwareSerial.h";
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>


//#include "SSD1306.h"


// Replace with your network credentials (STATION)
const char* ssid     = "JeffreysHotspot";
const char* password = "JeffreyRocks01";

unsigned long lastWifiPoll = 0;
unsigned long pollWifiLimit = 3000;

unsigned long lastGPSPoll = 0;
unsigned long pollGPSLimit = 3000;

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

//enum WIFI_STATUS {
//   WL_CONNECTED,
//   WIFI_CONNECTED 
//};

typedef struct{
   GPS_STATUS gps_status = GPS_DISCONNECTED;
   uint8_t wifi_status = WL_DISCONNECTED;
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
  
  pinMode(BUTTON_PIN_INTERRUPT, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN_INTERRUPT, buttonPress, FALLING);

  pinMode(button_1.getPIN(), INPUT_PULLUP);
  pinMode(button_2.getPIN(), INPUT_PULLUP);
  pinMode(button_3.getPIN(), INPUT_PULLUP);
  pinMode(button_4.getPIN(), INPUT_PULLUP);
  pinMode(button_5.getPIN(), INPUT_PULLUP);

  initializeLEDs();
  
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  lastGPSPoll = millis() + 1500;
  initWiFi();
  lastWifiPoll = millis();
}

void initializeLEDs(){
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  for (uint8_t nPix = 0; nPix < LED_COUNT; nPix++){
    strip.setPixelColor(0, 200, 0, 0);
  } //end for
  strip.show(); // Update strip with new contents
}

void initWiFi() {

  uint8_t wifiAttemptLimit = 7;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  tracker_status.wifi_status = WiFi.status();
  while ((tracker_status.wifi_status != WL_CONNECTED) && (wifiAttemptLimit-- > 0)) {
    Serial.print('.');
    delay(1000);
  }

  if ( tracker_status.wifi_status == WL_CONNECTED){
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Wifi failed to connect to "); 
    Serial.print(ssid);
  }
    
  updateWifiStatusLeds();
}

void checkWifiStatus() {

    tracker_status.wifi_status = WiFi.status();
  
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        tracker_status.wifi_status = WiFi.status();
   }
   updateWifiStatusLeds();
}

void updateWifiStatusLeds() {
  switch (tracker_status.wifi_status) {
     case WL_DISCONNECTED:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(red, 255, 120)));
       strip.setPixelColor(LED_WIFI_STATUS, 200, 0, 0);
       break;
     case WL_CONNECTED:
       //strip.setPixelColor(LED_GPS_STATUS, strip.gamma32(strip.ColorHSV(green, 255, 120)));
       strip.setPixelColor(LED_WIFI_STATUS, 0, 200, 0);
       break;
     default:
       strip.setPixelColor(LED_WIFI_STATUS, 0, 0, 200);
       break;
  }

  strip.show();
}

void checkGPSStatus() {

   bool gpsDataReady = false;
   static uint8_t gpsDataFails = 0;
    
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

      gpsDataFails = 0;

     if ((dLatitude == 0) || dLongitude == 0) {
       tracker_status.gps_status = GPS_CONNECTED_NO_DATA;
     }
     else {
       tracker_status.gps_status = GPS_CONNECTED;
     }
   } 
   else if (gpsDataFails++ > 1){
     tracker_status.gps_status = GPS_DISCONNECTED;
     gpsDataFails = 0;
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
       strip.setPixelColor(LED_GPS_STATUS, 200,0,0);
       break;
  }

  strip.show();
}

void gpsLogDump(){
  if (gps.location.isUpdated()){
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);
  }
  else if (gps.date.isUpdated()){
    Serial.print(F("DATE       Fix Age="));
    Serial.print(gps.date.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.date.value());
    Serial.print(F(" Year="));
    Serial.print(gps.date.year());
    Serial.print(F(" Month="));
    Serial.print(gps.date.month());
    Serial.print(F(" Day="));
    Serial.println(gps.date.day());
  }
  else if (gps.time.isUpdated()){
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  }
  else if (gps.speed.isUpdated()){
    Serial.print(F("SPEED      Fix Age="));
    Serial.print(gps.speed.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.speed.value());
    Serial.print(F(" Knots="));
    Serial.print(gps.speed.knots());
    Serial.print(F(" MPH="));
    Serial.print(gps.speed.mph());
    Serial.print(F(" m/s="));
    Serial.print(gps.speed.mps());
    Serial.print(F(" km/h="));
    Serial.println(gps.speed.kmph());
  }
  else if (gps.course.isUpdated()){
    Serial.print(F("COURSE     Fix Age="));
    Serial.print(gps.course.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.course.value());
    Serial.print(F(" Deg="));
    Serial.println(gps.course.deg());
  }
  else if (gps.altitude.isUpdated()){
    Serial.print(F("ALTITUDE   Fix Age="));
    Serial.print(gps.altitude.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.altitude.value());
    Serial.print(F(" Meters="));
    Serial.print(gps.altitude.meters());
    Serial.print(F(" Miles="));
    Serial.print(gps.altitude.miles());
    Serial.print(F(" KM="));
    Serial.print(gps.altitude.kilometers());
    Serial.print(F(" Feet="));
    Serial.println(gps.altitude.feet());
  }
  else if (gps.satellites.isUpdated()){
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());
  }
  else if (gps.hdop.isUpdated()){
    Serial.print(F("HDOP       Fix Age="));
    Serial.print(gps.hdop.age());
    Serial.print(F("ms raw="));
    Serial.print(gps.hdop.value());
    Serial.print(F(" hdop="));
    Serial.println(gps.hdop.hdop());
  }  
}

void loop() {

    if (millis() > (lastWifiPoll + pollWifiLimit)) {
      checkWifiStatus();
      lastWifiPoll = millis();
    }

    
    if (millis() > (lastGPSPoll + pollGPSLimit)) {
      checkGPSStatus();
      lastGPSPoll = millis();
    }
 
   if (button_1.readButtonState()) { //Panic Button
     Serial.print("LAT="); Serial.println(dLatitude, 6);
     Serial.print("LONG="); Serial.println(dLongitude, 6);
     Serial.print("ALT="); Serial.println(dAltitude, 2);
     Serial.print("Sats="); Serial.println(mNumSat);
   }
   else if (button_2.readButtonState()) { //All clear Button
     Serial.println("button 2 pressed");
   }
   else if (button_3.readButtonState()) {
     gpsLogDump();
     Serial.println("button 3 pressed");
   }
   else if (button_4.readButtonState()) {
     Serial.println("button 4 pressed");
   }
   else if (button_5.readButtonState()) {
     Serial.println("button 5 pressed");
   }
}
