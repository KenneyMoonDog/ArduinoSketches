#include "Arduino.h"
#include <ShipOperations.h>

ShipOperations::ShipOperations() {
  setupSound();
}

bool ShipOperations::readCurrentShipState(byte pinset) {
  return (bitRead(EN1701A::suiCurrentShipState, pinset));
  //return EN1701A::suiCurrentShipState & ( 0x1 << pinset );
}

bool ShipOperations::readOldShipState(byte pinset) {
  return (bitRead(EN1701A::suiPreviousShipState, pinset));
  //return EN1701A::suiPreviousShipState & ( 0x1 << pinset );
}

void ShipOperations::clearAll(){
  EN1701A::suiCurrentShipState = 0;
  EN1701A::suiPreviousShipState = 0;

  updateSection_DataRegister();
  digitalWrite(PIN_PHASER, LOW);
  digitalWrite(PIN_TORPEDO, LOW);
}

void ShipOperations::updateSection_DataRegister()
{
   digitalWrite(PIN_SR_LATCH, LOW);
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (EN1701A::suiCurrentShipState & 0xFF));
   shiftOut(PIN_SR_SECTION_DATA, PIN_SR_CLOCK, LSBFIRST, (EN1701A::suiCurrentShipState & 0xFF0000) >> 16);
   digitalWrite(PIN_SR_LATCH, HIGH);
}

void ShipOperations::ApplyShipLogic() {

  if (readCurrentShipState(POWER_CHANGE)) {
    if (readCurrentShipState(PRIMARY_SYSTEMS)) { //shutdown
      EN1701A::svWriteShipState(false, PRIMARY_SYSTEMS);
      EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_DOWN;
      clearAll();
    }
    else { //startup
      EN1701A::svWriteShipState(true, PRIMARY_SYSTEMS);
      EN1701A::suiCurrentShipState |= 0xFF & rand() % 254 + 1;
      EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_UP;
      updateSection_DataRegister();
    }
    playFile();
    EN1701A::svWriteShipState(false, POWER_CHANGE);
    return;
  }

  if (readCurrentShipState(PRIMARY_SYSTEMS) && !readCurrentShipState(SR_PHASER)){
    updateSection_DataRegister();
  }

  if (EN1701A::sbAudioIndex == AUDIO_INDEX_CANCEL) {
    //Serial.println("ShipOperations::Play Audio CANCEL");
    stopPlaying();
  }

  if (readCurrentShipState(AUDIO_EFFECT)) {
    playFile();
    EN1701A::svWriteShipState(false, AUDIO_EFFECT);
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

  if (readCurrentShipState(SR_PHASER)){
    if ( strcmp (pCurrentFilePlaying, scAudioEffects[AUDIO_INDEX_PHASER]) != 0) {
      EN1701A::sbAudioIndex = AUDIO_INDEX_PHASER;
      playFile();
      //flash PHASER lights
      delay(500);
      //digitalWrite(PIN_PHASER, HIGH);
      updateSection_DataRegister();
    }
  }
  else if (readOldShipState(SR_PHASER)){
    stopPlaying();
    //digitalWrite(PIN_PHASER, LOW);
    //EN1701A::svWriteShipState(false, PHASER);
    EN1701A::svWriteShipState(false, SR_PHASER);
    updateSection_DataRegister();
  }
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

void ShipOperations::audioCheck() {
  if (readCurrentShipState(PRIMARY_SYSTEMS)){
    if (!wave.isplaying) {
       //Serial.println(F("NO WAVE PLAYING"));
       EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_UP;
       playFile();
    }
  }
}

// Plays a full file from beginning to end with no pause.
void ShipOperations::playFile() {

  pCurrentFilePlaying = NULL;

  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    stopPlaying();
  }
  // look in the root directory and open the file
  if (!f.open(root, scAudioEffects[EN1701A::sbAudioIndex])) {
    putstring("Couldn't open file ");
    Serial.print(scAudioEffects[EN1701A::sbAudioIndex]);
    return;
  }

  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV");
    return;
  }

  pCurrentFilePlaying = scAudioEffects[EN1701A::sbAudioIndex];
  // ok time to play! start playback
  wave.play();
}
