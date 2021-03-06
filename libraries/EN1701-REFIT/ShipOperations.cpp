#include "Arduino.h"
#include <ShipOperations.h>
#include <EN1701-REFIT.h>

//#define PIN_CONSOLE_LIGHT 9

#define stop_impulse 0
#define half_impulse 85
#define full_impulse 255

byte impulseLevel[] = {SERIAL_COMM_IMPULSE_DRIVE, half_impulse};

ShipOperations::ShipOperations() {
  //pinMode(PIN_CONSOLE_LIGHT, OUTPUT);
  //digitalWrite(PIN_CONSOLE_LIGHT, HIGH);
  //clearAll();
  setupSound();
}

bool ShipOperations::readCurrentShipState(byte pinset) {
  return (bitRead(EN1701A::suiCurrentShipState, pinset));
}

bool ShipOperations::readOldShipState(byte pinset) {
  return (bitRead(EN1701A::suiPreviousShipState, pinset));
}

void ShipOperations::clearAll(){
  EN1701A::suiCurrentShipState = 0;
  EN1701A::suiPreviousShipState = 0;
  EN1701A::b_warp_mode_on = false;
  EN1701A::b_red_alert_on = false;
  EN1701A::b_phaser_on = false;
  EN1701A::b_power_cycle = false;
  impulseLevel[1] = half_impulse;
}

void ShipOperations::setImpulseLevel(byte level) {
   impulseLevel[1] = level;
   Serial.write(impulseLevel, 2);
}

void ShipOperations::increaseImpulseDrive(){
  switch (impulseLevel[1]){
    case stop_impulse:
      setImpulseLevel(half_impulse);
      break;
    case half_impulse:
      setImpulseLevel(full_impulse);
      break;
    case full_impulse:
      break;
    default:
      break;
  }
}

void ShipOperations::decreaseImpulseDrive(){
  switch (impulseLevel[1]){
    case stop_impulse:
      break;
    case half_impulse:
      setImpulseLevel(stop_impulse);
      break;
    case full_impulse:
      setImpulseLevel(half_impulse);
      break;
    default:
      break;
  }
}

void ShipOperations::ApplyShipLogic() {

  if (readCurrentShipState(POWER_CHANGE)) {
    if (readCurrentShipState(PRIMARY_SYSTEMS)) { //shutdown
      EN1701A::svWriteShipState(false, PRIMARY_SYSTEMS);
      Serial.write(SERIAL_COMM_POWER_OFF);
      EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_DOWN;
      clearAll();
    }
    else { //startup
      clearAll();
      EN1701A::svWriteShipState(true, PRIMARY_SYSTEMS);
      Serial.write(SERIAL_COMM_POWER_ON);
      EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_UP;
    }
    playFile();
    EN1701A::svWriteShipState(false, POWER_CHANGE);
    return;
  }

  if (!readCurrentShipState(PRIMARY_SYSTEMS)) {
    //clearAll();
    return;
  }

  if (readCurrentShipState(AUDIO_EFFECT)) {
    playFile();
    EN1701A::svWriteShipState(false, AUDIO_EFFECT);
    return;
  }

  if (EN1701A::sbAudioIndex == AUDIO_INDEX_CANCEL) {
    stopPlaying();
  }

  if (readCurrentShipState(TORPEDO)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_TORPEDO;
    playFile();
    Serial.write(SERIAL_COMM_TORPEDO);
    EN1701A::svWriteShipState(false, TORPEDO);
    return;
  }

  if(readCurrentShipState(SWITCH_TO_WARP_MODE)){
    setImpulseLevel(0);
    EN1701A::sbAudioIndex = AUDIO_INDEX_BTS1;
    playFile();
    //setTimer
    delay(3000);
    Serial.write(SERIAL_COMM_START_WARP_DRIVE);
    EN1701A::svWriteShipState(false, SWITCH_TO_WARP_MODE);
    EN1701A::svWriteShipState(false, SWITCH_TO_IMPULSE_MODE);
    return;
  }

  if(readCurrentShipState(SWITCH_TO_IMPULSE_MODE)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_BTS1;
    playFile();
    //setTimer
    delay(3000);
    setImpulseLevel(half_impulse);
    Serial.write(SERIAL_COMM_STOP_WARP_DRIVE);
    EN1701A::svWriteShipState(false, SWITCH_TO_WARP_MODE);
    EN1701A::svWriteShipState(false, SWITCH_TO_IMPULSE_MODE);
    return;
  }

  if(readCurrentShipState(INCREASE_IMPULSE_ENGINES)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_UP;
    playFile();
    increaseImpulseDrive();
    EN1701A::svWriteShipState(false, INCREASE_IMPULSE_ENGINES);
    return;
  }

  if(readCurrentShipState(DECREASE_IMPULSE_ENGINES)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_DOWN;
    playFile();
    decreaseImpulseDrive();
    EN1701A::svWriteShipState(false, DECREASE_IMPULSE_ENGINES);
    return;
  }

  if(readCurrentShipState(INCREASE_WARP_ENGINES)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_UP;
    playFile();
    Serial.write(SERIAL_COMM_INCREASE_WARP_DRIVE);
    EN1701A::svWriteShipState(false, INCREASE_WARP_ENGINES);
    return;
  }

  if(readCurrentShipState(DECREASE_WARP_ENGINES)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_DOWN;
    playFile();
    Serial.write(SERIAL_COMM_DECREASE_WARP_DRIVE);
    EN1701A::svWriteShipState(false, DECREASE_WARP_ENGINES);
    return;
  }

  if (readCurrentShipState(PHASER_ON)){
    EN1701A::sbAudioIndex = AUDIO_INDEX_PHASER;
    playFile();
    delay(300);
    Serial.write(SERIAL_COMM_PHASER_ON);
    EN1701A::svWriteShipState(false, PHASER_ON);
    EN1701A::svWriteShipState(false, PHASER_OFF);
  }
  else if (readCurrentShipState(PHASER_OFF)){
    stopPlaying();
    Serial.write(SERIAL_COMM_PHASER_OFF);
    EN1701A::svWriteShipState(false, PHASER_ON);
    EN1701A::svWriteShipState(false, PHASER_OFF);
  }

  return;
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
    //putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
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
  //  putstring_nl("No valid FAT partition!");
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
  //  putstring_nl("Can't open root dir!"); // Something went wrong,
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
       EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_CONTINUE;
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
//    putstring("Couldn't open file ");
//    Serial.print(scAudioEffects[EN1701A::sbAudioIndex]);
    return;
  }

  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
//    putstring_nl("Not a valid WAV");
    return;
  }

  pCurrentFilePlaying = scAudioEffects[EN1701A::sbAudioIndex];
  // ok time to play! start playback
  wave.play();
}
