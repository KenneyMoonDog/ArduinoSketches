#include "SPI.h"
#include "SD.h"

#define SD_CLK 13
#define SD_MISO 12
#define SD_MOSI 11
#define SD_CS A2

void setup() {
  Serial.begin(115200);
  SPI.begin();

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  if(!SD.begin(SD_CS)) {
    Serial.println("FAILED");
    while (true){
      ;
    }
  }
  else {
    Serial.println("SD CARD is OK!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
