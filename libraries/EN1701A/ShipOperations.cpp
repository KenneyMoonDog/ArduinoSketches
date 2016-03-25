#include "Arduino.h"
#include <ShipOperations.h>

ShipOperations::ShipOperations() {
  setupSound();
}

bool ShipOperations::readCurrentShipState(unsigned int pinset) {
  return (bitRead(EN1701A::suiCurrentShipState, pinset));
}

bool ShipOperations::readOldShipState(unsigned int pinset) {
  return (bitRead(EN1701A::suiPreviousShipState, pinset));
}

void ShipOperations::clearAll(){
  EN1701A::suiCurrentShipState = 0;
  EN1701A::suiPreviousShipState = 0;

  updateSectionDataRegister();
  digitalWrite(PIN_PHASER, LOW);
  digitalWrite(PIN_TORPEDO, LOW);
}

void ShipOperations::updateSectionDataRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (EN1701A::suiCurrentShipState & 0xFF));
   digitalWrite(PIN_SR_LATCH, HIGH);
}

void ShipOperations::ApplyShipLogic() {

  if (readCurrentShipState(POWER_CHANGE)) {
    if (readCurrentShipState(PRIMARY_SYSTEMS)) { //shutdown
      EN1701A::svWriteShipState(false, PRIMARY_SYSTEMS);
      //playFile(scAudioEffects[AUDIO_INDEX_POWER_DOWN]);
      EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_DOWN;
      clearAll();
    }
    else { //startup
      EN1701A::svWriteShipState(true, PRIMARY_SYSTEMS);
      EN1701A::suiCurrentShipState |= 0xFF & rand() % 254 + 1;
      //playFile(scAudioEffects[AUDIO_INDEX_POWER_UP]);
      EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_UP;
      updateSectionDataRegister();
      //TODO: write out to the appropriate lighting ports
            //analogWrite(6,240);
    }
    playFile();
    EN1701A::svWriteShipState(false, POWER_CHANGE);
    return;
  }

  if (readCurrentShipState(PRIMARY_SYSTEMS)){
    updateSectionDataRegister();
  }

  if (readCurrentShipState(AUDIO_EFFECT)) {
    //Serial.println("ShipOperations::Play Audio effect");
    if (EN1701A::sbAudioIndex == AUDIO_INDEX_CANCEL) {
      //Serial.println("ShipOperations::Play Audio CANCEL");
      stopPlaying();
    }
    else {
      //Serial.println("ShipOperations::Play Audio Index:");
      //Serial.println(scAudioEffects[EN1701A::sbAudioIndex]);
      playFile();
    }
    EN1701A::svWriteShipState(false, AUDIO_EFFECT);
    //return;
  }

  if (readCurrentShipState(TORPEDO)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_TORPEDO;
    playFile();

    //flash torpedo lights
    delay(100);
    digitalWrite(PIN_TORPEDO, HIGH);
    delay(100);
    digitalWrite(PIN_TORPEDO, LOW);
    EN1701A::svWriteShipState(false, TORPEDO);
    return;
  }

  if (readCurrentShipState(PHASER)){
    if ( strcmp (pCurrentFilePlaying, scAudioEffects[AUDIO_INDEX_PHASER]) != 0) {
      EN1701A::sbAudioIndex = AUDIO_INDEX_PHASER;
      playFile();
      //flash PHASER lights
      delay(500);
      digitalWrite(PIN_PHASER, HIGH);
    }
  }
  else if (readOldShipState(PHASER)){
    stopPlaying();
    digitalWrite(PIN_PHASER, LOW);
  }
  /*if (readCurrentShipState(PHASER)){
    if ( strcmp (pCurrentFilePlaying, scAudioEffects[AUDIO_INDEX_PHASER]) != 0) {
      Serial.println(F("ShipOperations::START PHASER"));
      playFile(scAudioEffects[AUDIO_INDEX_PHASER]);
      //flash PHASER lights
      delay(500);
      digitalWrite(PIN_PHASER, HIGH);
      EN1701A::svWriteShipState(true, PHASER);
    }
  } else if (readOldShipState(PHASER)){
    Serial.println(F("ShipOperations::STOP PHASER"));
    stopPlaying();
    digitalWrite(PIN_PHASER, LOW);
    EN1701A::svWriteShipState(false, PHASER);
  }*/
}

void ShipOperations::setupSound() {
  //putstring_nl("WaveHC with 6 buttons");

  //putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  //Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!

  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    //sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }

  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part))
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    //sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }

  // Lets tell the user about what we found
  /*putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?*/

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }

  // Whew! We got past the tough parts.
  //putstring_nl("Ready!");
}

// this handy function will return the number of bytes currently free in RAM, great for debugging!
int ShipOperations::freeRam(void)
{
  extern int  __bss_end;
  extern int  *__brkval;
  int free_memory;
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  return free_memory;
}

void ShipOperations::stopPlaying(){
  pCurrentFilePlaying = NULL;
  wave.stop();
}
/*void ShipOperations::sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}*/

// Plays a full file from beginning to end with no pause.
/*void ShipOperations::playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
}*/
//void ShipOperations::playFile(char *name) {
void ShipOperations::playFile() {

  pCurrentFilePlaying = scAudioEffects[EN1701A::sbAudioIndex];;

  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    stopPlaying();
  }
  // look in the root directory and open the file
  if (!f.open(root, pCurrentFilePlaying)) {
    putstring("Couldn't open file "); Serial.print(pCurrentFilePlaying);
    return;
  }

  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV");
    return;
  }

  //pCurrentFilePlaying = name;
  // ok time to play! start playback
  wave.play();
}
