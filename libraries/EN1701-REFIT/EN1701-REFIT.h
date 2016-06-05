#ifndef EN1701-REFIT_h
#define EN1701-REFIT_h

#include "Arduino.h"
#include "SERIAL_COMM.h"

/*This set maps bits in an unsigned int to
an illuminated control section of the ship*/
#define SECTION_0 0      //byte1
#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3
#define SECTION_4 4
#define SECTION_5 5
#define SECTION_6 6
#define SECTION_7 7
//#define SECTION_8 8
//#define SECTION_9 9
//#define SECTION_10 10
//#define SECTION_11 11
//#define SECTION_12 12
//#define SECTION_13 13
#define SR_PHASER 14  //byte2
#define SR_TORPEDO 15

#define PRIMARY_SYSTEMS 16   //byte3
#define RUNNING_LIGHTS 17
//#define PHASER 10

#define WARP_ENGINES 18
#define IMPULSE_ENGINES 19
#define POWER_CHANGE 20
#define AUDIO_EFFECT 21
//#define SR_PHASER 16
//#define SR_TORPEDO 17                    //byte3


#define AUDIO_INDEX_CANCEL 100
#define AUDIO_INDEX_RED_ALERT 0
#define AUDIO_INDEX_P1_MESSAGE 1
#define AUDIO_INDEX_TORPEDO 2
#define AUDIO_INDEX_PHASER 3
#define AUDIO_INDEX_POWER_DOWN 4
#define AUDIO_INDEX_POWER_UP 5
#define AUDIO_INDEX_DESTRUCT 6
#define AUDIO_INDEX_HIT 7

/*This set maps PIN assignemnts to bit locations*/
//define PIN_SR_CLOCK 14
//#define PIN_SR_LATCH 15
//#define PIN_SR_SECTION_DATA 16

//#define PIN_SR_SECTION_ENABLE 6
#define PIN_IR_RECEIVER 11

//#define POLLING_FREQUENCY 250

static char* scAudioEffects[]={"KLAX.WAV", "P1MSG.WAV", "TORP1.WAV", "SPZER1.WAV", "BPD1.WAV", "BPUP1.WAV", "DSTRT.WAV", "HULHIT.WAV"};

class EN1701A
{
  public:
    static byte sendByte;
    static byte sbAudioIndex;
    static unsigned long suiCurrentShipState;
    static unsigned long suiPreviousShipState;
    static void svWriteShipState(bool set, byte pinset );
};

#endif
