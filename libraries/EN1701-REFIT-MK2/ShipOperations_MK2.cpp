#include "Arduino.h"
#include <ShipOperations_MK2.h>
#include <EN1701-REFIT_MK2.h>
#include <ButtonReader_MK2.h>

#define PIN_CONSOLE_LIGHT 9

#define stop_impulse 0
#define half_impulse 60
#define full_impulse 255

byte impulseLevel[] = {SERIAL_COMM_IMPULSE_DRIVE, half_impulse};
byte saucerSection[] = {0,0};
byte setColor[] = {0,0,0,0};

ShipOperations_MK2::ShipOperations_MK2() {
  pinMode(PIN_CONSOLE_LIGHT, OUTPUT);
  switchConsoleMode(MODE_HELM);
  setupSound();
}

void ShipOperations_MK2::clearAll(){
  EN1701A::buttonPressed = 0;
  EN1701A::b_warp_mode_on = false;

  if (EN1701A::b_warp_engine_on){
     Serial.write(SERIAL_COMM_STOP_WARP_DRIVE);
     EN1701A::b_warp_engine_on=false;
  }

  if (EN1701A::b_red_alert_on){
     Serial.write(SERIAL_COMM_RED_ALERT_OFF);
     EN1701A::b_red_alert_on = false;
  }

  if (EN1701A::b_phaser_on){
    Serial.write(SERIAL_COMM_PHASER_OFF);
    EN1701A::b_phaser_on = false;
  }

  if (EN1701A::b_power_cycle){
    EN1701A::b_power_cycle = false;
    Serial.write(SERIAL_COMM_POWER_OFF);
  }

  setImpulseLevel(stop_impulse);
}

void ShipOperations_MK2::setTargetColor(byte target, byte* color) {
   setColor[0] = target;
   setColor[1] = color[0];
   setColor[2] = color[1];
   setColor[3] = color[2];
   Serial.write(setColor,4);
}

void ShipOperations_MK2::setShipSection(byte section, byte set, int waitMs) {
   saucerSection[0] = section;
   saucerSection[1] = set;
   Serial.write(saucerSection, 2);
   delay(waitMs);
}

void ShipOperations_MK2::setImpulseLevel(byte level) {
   impulseLevel[1] = level;
   Serial.write(impulseLevel, 2);
}

void ShipOperations_MK2::switchConsoleMode(byte mode) {
   EN1701A::console_mode = mode;
   switch (mode) {
     case MODE_HELM:
       switchConsoleLight(true);
       EN1701A::sbAudioIndex = AUDIO_INDEX_TLIFT_BRIDGE;
       //EN1701A::b_power_cycle = true;
       playFile();
       break;
     case MODE_AUX_CONTROL:
       switchConsoleLight(false);
       EN1701A::sbAudioIndex = AUDIO_INDEX_TLIFT_AUXCONTROL;
       playFile();
       break;
     case MODE_ENGINEERING:
       switchConsoleLight(false);
       EN1701A::sbAudioIndex = AUDIO_INDEX_TLIFT_ENGINEERING;;
       playFile();
       break;
     default:
       switchConsoleLight(true);
       break;
   }
}

void ShipOperations_MK2::switchConsoleLight(boolean bOn){
  if (bOn) {
    digitalWrite(PIN_CONSOLE_LIGHT, HIGH);
  }
  else {
    digitalWrite(PIN_CONSOLE_LIGHT, LOW);
  }
  EN1701A::console_light_state = bOn;
}

void ShipOperations_MK2::increaseImpulseDrive(){
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

void ShipOperations_MK2::decreaseImpulseDrive(){
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

void ShipOperations_MK2::ApplyShipLogic() {

  if ( EN1701A::console_mode == MODE_ENGINEERING) {

     switch (EN1701A::buttonPressed) {
       case PIN_A_BUTTON: //red alert
         EN1701A::buttonPressed = 0;

         if (!EN1701A::b_red_alert_on){
           EN1701A::sbAudioIndex = AUDIO_INDEX_RED_ALERT;
           playFile();
           delay(300);
           Serial.write(SERIAL_COMM_RED_ALERT_ON);
           EN1701A::b_red_alert_on = true;
         }
         else {
           stopPlaying();
           Serial.write(SERIAL_COMM_RED_ALERT_OFF);
           EN1701A::b_red_alert_on = false;
         }
         break;
       case PIN_B_BUTTON: //large beam out
         EN1701A::buttonPressed = 0;
         EN1701A::sbAudioIndex = AUDIO_INDEX_MODE_T_SHORT_TRANSPORT;
         playFile();
         break;
       case PIN_C_BUTTON: //mode up
         EN1701A::buttonPressed = 0;
         switchConsoleMode(MODE_AUX_CONTROL);
         break;
       case PIN_D_BUTTON: //??
         EN1701A::buttonPressed = 0;
         EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
         playFile();
         break;
       case PIN_E_BUTTON: //??
         EN1701A::buttonPressed = 0;
         EN1701A::sbAudioIndex = AUDIO_INDEX_P1_MESSAGE;
         playFile();
         delay(200);
         break;
       case PIN_F_BUTTON: //night mode
         EN1701A::buttonPressed = 0;
         EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
         playFile();
         break;
       case PIN_G_BUTTON: //mode down
         EN1701A::buttonPressed = 0;
         switchConsoleMode(MODE_HELM);
         break;
       case PIN_H_BUTTON: //destruct
         EN1701A::buttonPressed = 0;
         EN1701A::sbAudioIndex = AUDIO_INDEX_DSTRT;
         playFile();
         delay(200);
         break;
       default:
         EN1701A::buttonPressed = 0;
         break;
     }

     return;
  }

  if ( EN1701A::console_mode == MODE_AUX_CONTROL) {
    switch (EN1701A::buttonPressed) {
      case PIN_A_BUTTON:
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        break;
      case PIN_B_BUTTON:  //LMIX
        //add button press
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        break;
      case PIN_C_BUTTON: //MODE UP
        //add button press
        EN1701A::buttonPressed = 0;
        switchConsoleMode(MODE_HELM);
        break;
      case PIN_D_BUTTON: //auto destruct ENGAGED
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        break;
      case PIN_E_BUTTON:   //PI message
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        break;
      case PIN_F_BUTTON: //THEME
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        break;
      case PIN_G_BUTTON: //mode down
        //add button press
        EN1701A::buttonPressed = 0;
        switchConsoleMode(MODE_ENGINEERING);
        break;
      case PIN_H_BUTTON:  //??
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        delay(200);
        break;
      default:
        EN1701A::buttonPressed = 0;
        break;
    }
    return;
  }

  if ( EN1701A::console_mode == MODE_HELM) {
    switch (EN1701A::buttonPressed) {
      case PIN_A_BUTTON:
        EN1701A::buttonPressed = 0;
        if ( EN1701A::b_power_cycle) { //shutdown
          EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_DOWN;
          clearAll();
          playFile();

          delay(500);

          setShipSection(SERIAL_COMM_FLOOD_1,0,500);
          setShipSection(SERIAL_COMM_FLOOD_2,0,500);
          Serial.write(SERIAL_COMM_FLOOD_ENGINE_OFF);
          delay(500);
          Serial.write(SERIAL_COMM_FLOOD_FORWARD_OFF);
          delay(500);
          setShipSection(SERIAL_COMM_AFT_SECTION,0,500);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_LOUNGE,0,500);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_1,0,0);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_6,0,750);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_2,0,0);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_5,0,750);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_3,0,0);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_4,0,750);

          setShipSection(SERIAL_COMM_NECK_SECTION,0,500);
          setShipSection(SERIAL_COMM_ARBORITUM,0,500);
          setShipSection(SERIAL_COMM_BELLY_SECTION,0,500);
          setShipSection(SERIAL_COMM_HANGER_SECTION,0,500);

          setShipSection(SERIAL_COMM_ENGINEERING_SECTION_1,0,500);
          setShipSection(SERIAL_COMM_ENGINEERING_SECTION_2,0,500);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_BRIDGE,0,750);

          setTargetColor(SERIAL_COMM_DEFLECTOR_COLOR, colorOff);
          setTargetColor(SERIAL_COMM_NACELLE_COLOR, colorWhite);
          setTargetColor(SERIAL_COMM_CRYSTAL_COLOR, colorWhite);
        }
        else {
          Serial.write(SERIAL_COMM_POWER_ON);
          EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_UP;
          EN1701A::b_power_cycle = true;
          playFile();

          setTargetColor(SERIAL_COMM_CRYSTAL_COLOR, colorAmber);
          setTargetColor(SERIAL_COMM_NACELLE_COLOR, colorBlue);
          delay(750);

          Serial.write(SERIAL_COMM_RED_ALERT_OFF);
          setShipSection(SERIAL_COMM_ENGINEERING_SECTION_2,1,750);
          setShipSection(SERIAL_COMM_ENGINEERING_SECTION_1,1,750);
          setShipSection(SERIAL_COMM_BELLY_SECTION,1,500);
          setShipSection(SERIAL_COMM_HANGER_SECTION,1,500);
          setShipSection(SERIAL_COMM_NECK_SECTION,1,500);

          setShipSection(SERIAL_COMM_SAUCER_SECTION_LOUNGE,1,500);

          setShipSection(SERIAL_COMM_SAUCER_SECTION_1,1,0);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_6,1,500);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_2,1,0);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_5,1,500);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_3,1,0);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_4,1,500);
          setShipSection(SERIAL_COMM_SAUCER_SECTION_BRIDGE,1,500);
          setShipSection(SERIAL_COMM_ARBORITUM,1,500);
          setShipSection(SERIAL_COMM_AFT_SECTION,1,500);
          setShipSection(SERIAL_COMM_FLOOD_1,1,500);
          setShipSection(SERIAL_COMM_FLOOD_2,1,1000);

          Serial.write(SERIAL_COMM_FLOOD_ENGINE_ON);
          delay(500);
          Serial.write(SERIAL_COMM_FLOOD_FORWARD_ON);
          delay(500);

          setTargetColor(SERIAL_COMM_DEFLECTOR_COLOR, colorAmber);
        }

        break;

      case PIN_B_BUTTON: //speed up
        EN1701A::buttonPressed = 0;
        if (EN1701A::b_warp_mode_on) { //warp
          EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_UP;
          playFile();
          Serial.write(SERIAL_COMM_INCREASE_WARP_DRIVE);
          EN1701A::b_warp_engine_on = true;
        }
        else {  //impulse
          EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_UP;
          playFile();
          increaseImpulseDrive();
        }
        break;

      case PIN_C_BUTTON: //mode buttons up
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        switchConsoleMode(MODE_ENGINEERING);
        break;

      case PIN_D_BUTTON: //phaser
        EN1701A::buttonPressed = 0;
        if (!EN1701A::b_phaser_on){
          EN1701A::sbAudioIndex = AUDIO_INDEX_PHASER;
          playFile();
          delay(300);
          Serial.write(SERIAL_COMM_PHASER_ON);
          EN1701A::b_phaser_on = true;
        }
        else {
          stopPlaying();
          Serial.write(SERIAL_COMM_PHASER_OFF);
          EN1701A::b_phaser_on = false;
        }
        break;

      case PIN_E_BUTTON: //deflector mode
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS1;
        playFile();
        //setTimer
        delay(3000);

        if (EN1701A::b_warp_mode_on){
          setTargetColor(SERIAL_COMM_DEFLECTOR_COLOR, colorAmber);
          setTargetColor(SERIAL_COMM_CRYSTAL_COLOR, colorAmber);
          setImpulseLevel(half_impulse);
          if (EN1701A::b_warp_engine_on){
            Serial.write(SERIAL_COMM_STOP_WARP_DRIVE);
            EN1701A::b_warp_engine_on = false;
          }
        }
        else {
          setTargetColor(SERIAL_COMM_DEFLECTOR_COLOR, colorBlue);
          setTargetColor(SERIAL_COMM_CRYSTAL_COLOR, colorBlue);
          setImpulseLevel(stop_impulse);
          //Serial.write(SERIAL_COMM_START_WARP_DRIVE);
          //EN1701A::b_warp_engine_on = true;
        }
        EN1701A::b_warp_mode_on = !EN1701A::b_warp_mode_on;
        break;

      case PIN_F_BUTTON: //speed down
        EN1701A::buttonPressed = 0;
        if(EN1701A::b_warp_mode_on){
          EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_DOWN;
          playFile();
          Serial.write(SERIAL_COMM_DECREASE_WARP_DRIVE);
        }
        else {
          EN1701A::sbAudioIndex = AUDIO_INDEX_WARP_DOWN;
          playFile();
          decreaseImpulseDrive();
        }
        break;

      case PIN_G_BUTTON: //mode down
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_BTS6;
        playFile();
        switchConsoleMode(MODE_AUX_CONTROL);
        break;

      case PIN_H_BUTTON: //torpedos
        EN1701A::buttonPressed = 0;
        EN1701A::sbAudioIndex = AUDIO_INDEX_TORPEDO;
        playFile();
        Serial.write(SERIAL_COMM_TORPEDO);
        break;
      default:
        break;
    }
    return;
  }
}

void ShipOperations_MK2::setupSound() {
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
int ShipOperations_MK2::freeRam(void)
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

void ShipOperations_MK2::stopPlaying(){
  pCurrentFilePlaying = NULL;
  wave.stop();
}
/*void ShipOperations_MK2::sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}*/

void ShipOperations_MK2::audioCheck() {
  if (EN1701A::b_power_cycle){
    if (!wave.isplaying) {
       switch (EN1701A::console_mode){
         case MODE_HELM:
           EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_CONTINUE;
           break;
         case MODE_AUX_CONTROL:
           EN1701A::sbAudioIndex = AUDIO_INDEX_AUXCONTROL_CONTINUE;
           break;
         case MODE_ENGINEERING:
           EN1701A::sbAudioIndex = AUDIO_INDEX_MODE_T_BACKGROUND;
           break;
         default:
           EN1701A::sbAudioIndex = AUDIO_INDEX_POWER_CONTINUE;
           break;
       }
       playFile();
    }
  }
}

// Plays a full file from beginning to end with no pause.
void ShipOperations_MK2::playFile() {

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
