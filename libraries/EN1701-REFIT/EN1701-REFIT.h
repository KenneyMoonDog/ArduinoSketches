#ifndef EN1701-REFIT_h
#define EN1701-REFIT_h

#include "Arduino.h"
#include "SERIAL_COMM.h"

/*This set maps bits in an unsigned int to
an illuminated control section of the ship*/
#define SR_RUNNING_LIGHTS_ 0  //byte 1
#define SR_RUNNING_LIGHTS_ 1
#define SR_RUNNING_LIGHTS_ 2
#define SR_RUNNING_LIGHTS_ 3
#define SR_RUNNING_LIGHTS_ 4
#define SR_MAIN_POWER 5
#define SR_PHASER 6
#define SR_TORPEDO 7
#define SR_SECTION_0 8      //byte2
#define SR_SECTION_1 9
#define SR_SECTION_2 10
#define SR_SECTION_3 11
#define SR_SECTION_4 12
#define SR_SECTION_5 13
#define SR_SECTION_6 14
#define SR_SECTION_7 15

#define PRIMARY_SYSTEMS 16
#define RUNNING_LIGHTS 17
#define WARP_ENGINES 18
#define IMPULSE_ENGINES 19
#define POWER_CHANGE 20
#define AUDIO_EFFECT 21

#define AUDIO_INDEX_CANCEL 100
#define AUDIO_INDEX_RED_ALERT 0
#define AUDIO_INDEX_P1_MESSAGE 1
#define AUDIO_INDEX_TORPEDO 2
#define AUDIO_INDEX_PHASER 3
#define AUDIO_INDEX_POWER_DOWN 4
#define AUDIO_INDEX_POWER_UP 5
#define AUDIO_INDEX_DESTRUCT 6
#define AUDIO_INDEX_HIT 7
#define AUDIO_INDEX_BTS1 8
#define AUDIO_INDEX_BTS2 9
#define AUDIO_INDEX_BTS3 10
#define AUDIO_INDEX_BT4 11
#define AUDIO_INDEX_BT5 12
#define AUDIO_INDEX_BT6 13

#define PIN_IR_RECEIVER 11


static char* scAudioEffects[]={"KLAX.WAV", "P1MSG.WAV", "TORP1.WAV", "SPZER1.WAV", "BPD1.WAV", "BPUP1.WAV", "DSTRT.WAV", "HULHIT.WAV", "BTS1.WAV", "BTS2.WAV", "BTS3.WAV", "BT4.WAV", "BT5.WAV", "BT6.WAV"};
static byte colorWhite[] = {10, 10, 10};
static byte colorAmber[] = {250, 85, 0};
static byte colorBlue[] = {0, 0, 255};
static byte colorOff[] = {0, 0, 0};

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
