#include "FS.h"
#include "SD.h"
#include "SPI.h"

void setup() {
   Serial.begin(115200);
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);  

  //if(!SD.begin(5)){
  //  Serial.println("Card Mount Failed");
  //  return;
  //}
}

void loop() {
  // put your main code here, to run repeatedly:
}
