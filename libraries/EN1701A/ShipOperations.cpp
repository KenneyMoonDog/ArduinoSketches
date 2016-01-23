#include "Arduino.h"
#include <ShipOperations.h>

ShipOperations::ShipOperations( unsigned int *currState, unsigned int *oldState ) {
  pCurrentShipState = currState;
  pOldShipState = oldState;
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
  mSectionData = 0;

  updateSectionDataRegister();
  //digitalWrite(PIN_PHASER, LOW);
  //digitalWrite(PIN_TORPEDO, LOW);
}

void ShipOperations::ApplyShipLogic() {

  //if (bitRead(*pCurrentShipState, POWER_CHANGE)) {
  //  if ( bitRead(*pCurrentShipState, PRIMARY_SYSTEMS)) { //shutdown
  if (readCurrentShipState(POWER_CHANGE)) {
    if (readCurrentShipState(PRIMARY_SYSTEMS)) { //shutdown
      writeShipState(false, PRIMARY_SYSTEMS);
      mSectionData = 0;
      playcomplete("BPD1.WAV");
      clearAll();
    }
    else { //startup
      writeShipState(true, PRIMARY_SYSTEMS);
      mSectionData = 0xFF;
      playcomplete("BPUP1.WAV");
      updateSectionDataRegister();
            //analogWrite(6,240);
    }
    writeShipState(false, POWER_CHANGE);

    return;
  }

  //if (bitRead(*pCurrentShipState, TORPEDO)){
  if (readCurrentShipState(TORPEDO)){
    playcomplete("TORP1.WAV");
    //flash torpedo lights
    delay(100);
    digitalWrite(PIN_TORPEDO, HIGH);
    delay(100);
    digitalWrite(PIN_TORPEDO, LOW);

    writeShipState(false, TORPEDO);
  }

  if (readCurrentShipState(PHASER)){
    playcomplete("SPZER1.WAV");
    //flash PHASER lights
    digitalWrite(PIN_PHASER, HIGH);
    //writeShipState(false, TORPEDO);
  } else if (readOldShipState(PHASER)){
    digitalWrite(PIN_PHASER, LOW);
    wave.stop();
    writeShipState(false, PHASER);
  }
}

void ShipOperations::cleanTimeouts(){

}
/*void ShipOperations::ApplyLights(){
  updateShiftRegister();
  //TODO: add in cases for other sections being enabled/disabled
}

void ShipOperations::ApplySounds(){
   if (bitRead(*pCurrentShipState, PRIMARY_SYSTEMS) &&
       !(bitRead(*pOldShipState, PRIMARY_SYSTEMS)){
      playcomplete("BPUP1.WAV");
   }
} */

void ShipOperations::updateSectionDataRegister()
{
   Serial.println("ShipOperations::SHIFTING IN:");
   Serial.println(mSectionData);

   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, mSectionData);
   digitalWrite(PIN_SR_LATCH, HIGH);
}

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
