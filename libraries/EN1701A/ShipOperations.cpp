#include "Arduino.h"
#include <ShipOperations.h>

ShipOperations::ShipOperations( unsigned int *currState, unsigned int *oldState, char* audioEffectFiles[], byte *audioIndex ) {
  pCurrentShipState = currState;
  pOldShipState = oldState;
  pAudioEffectFiles = audioEffectFiles;
  pAudioIndex = audioIndex;
  setupSound();
}

void ShipOperations::writeShipState(bool set, unsigned int pinset ){
  *pOldShipState = *pCurrentShipState;
  if (set) {
    bitSet(*pCurrentShipState, pinset);
  }
  else {
    bitClear(*pCurrentShipState, pinset);
  }
}

bool ShipOperations::readCurrentShipState(unsigned int pinset) {
  return (bitRead(*pCurrentShipState, pinset));
}

bool ShipOperations::readOldShipState(unsigned int pinset) {
  return (bitRead(*pOldShipState, pinset));
}

void ShipOperations::clearAll(){
  *pCurrentShipState = 0;
  *pOldShipState = 0;

  updateSectionDataRegister();
  digitalWrite(PIN_PHASER, LOW);
  digitalWrite(PIN_TORPEDO, LOW);
}

void ShipOperations::updateSectionDataRegister()
{
   Serial.println("ShipOperations::SHIFTING IN:");
   Serial.println(*pCurrentShipState & 0xFF);

   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (*pCurrentShipState & 0xFF));
   digitalWrite(PIN_SR_LATCH, HIGH);
}

void ShipOperations::ApplyShipLogic() {

  if (readCurrentShipState(POWER_CHANGE)) {
    if (readCurrentShipState(PRIMARY_SYSTEMS)) { //shutdown
      writeShipState(false, PRIMARY_SYSTEMS);
      playcomplete(pAudioEffectFiles[AUDIO_INDEX_POWER_DOWN]);
      clearAll();
    }
    else { //startup
      writeShipState(true, PRIMARY_SYSTEMS);
      *pCurrentShipState |= 0xFF;
      playcomplete(pAudioEffectFiles[AUDIO_INDEX_POWER_UP]);
      updateSectionDataRegister();
      //TODO: write out to the appropriate lighting ports
            //analogWrite(6,240);
    }
    writeShipState(false, POWER_CHANGE);
    return;
  }

  if (readCurrentShipState(AUDIO_EFFECT)) {
    //Serial.println("ShipOperations::Play Audio effect");
    if (*pAudioIndex == AUDIO_INDEX_CANCEL) {
      //Serial.println("ShipOperations::Play Audio CANCEL");
      wave.stop();
    }
    else {
      //Serial.println("ShipOperations::Play Audio Index:");
      //Serial.println(pAudioEffectFiles[*pAudioIndex]);
      playcomplete(pAudioEffectFiles[*pAudioIndex]);
    }
    writeShipState(false, AUDIO_EFFECT);
    return;
  }

  if (readCurrentShipState(PRIMARY_SYSTEMS)){

  }

  if (readCurrentShipState(TORPEDO)){
    playcomplete(pAudioEffectFiles[AUDIO_INDEX_TORPEDO]);
    //flash torpedo lights
    delay(100);
    digitalWrite(PIN_TORPEDO, HIGH);
    delay(100);
    digitalWrite(PIN_TORPEDO, LOW);
    writeShipState(false, TORPEDO);
    //return;
  }

  if (readCurrentShipState(PHASER)){
    Serial.println("ShipOperations::START PHASER");
    playcomplete(pAudioEffectFiles[AUDIO_INDEX_PHASER]);
    //flash PHASER lights
    delay(500);
    digitalWrite(PIN_PHASER, HIGH);
    writeShipState(true, PHASER);
    //return;
  } else if (readOldShipState(PHASER)){
    Serial.println("ShipOperations::STOP PHASER");
    wave.stop();
    digitalWrite(PIN_PHASER, LOW);
    writeShipState(false, PHASER);
    //return;
  }
}

/*void ShipOperations::ApplyLights(){
  updateShiftRegister();
  //TODO: add in cases for other sections being enabled/disabled
}*/

void ShipOperations::setupSound() {
  putstring_nl("WaveHC with 6 buttons");

  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!

  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
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
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }

  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }

  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
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

void ShipOperations::sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

// Plays a full file from beginning to end with no pause.
void ShipOperations::playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
}

void ShipOperations::playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }

  // ok time to play! start playback
  wave.play();
}
