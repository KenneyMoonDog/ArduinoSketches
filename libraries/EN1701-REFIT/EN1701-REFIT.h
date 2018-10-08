#ifndef EN1701-REFIT_h
#define EN1701-REFIT_h

#include "Arduino.h"
#include "SERIAL_COMM.h"

/*This set maps bits in an unsigned int to
an illuminated control section of the ship*/

#define SR_BUTTON_0 0 //byte 1
#define SR_BUTTON_1 1
#define SR_BUTTON_2 2
#define SR_BUTTON_3 3
#define SR_BUTTON_4 4
#define SR_BUTTON_5 5
#define SR_BUTTON_6 6
#define SR_BUTTON_7 7
#define SR_BUTTON_8 8 //byte2
//#define SR_BUTTON_9 9
//#define MAIN_POWER 10
#define MAIN_POWER 9
#define PHASER_OFF 10
#define PHASER_ON 11
#define TORPEDO 12
#define PRIMARY_SYSTEMS 13
#define RUNNING_LIGHTS 14
#define WARP_ENGINES 15
#define IMPULSE_ENGINES 16 //byte3
#define POWER_CHANGE 17
#define AUDIO_EFFECT 18
#define INCREASE_WARP_ENGINES 19
#define DECREASE_WARP_ENGINES 20
#define INCREASE_IMPULSE_ENGINES 19
#define DECREASE_IMPULSE_ENGINES 20
//#define PHASER_OFF 21

#define AUDIO_INDEX_CANCEL 100

#define AUDIO_INDEX_RED_ALERT 0
#define AUDIO_INDEX_P1_MESSAGE 1
#define AUDIO_INDEX_TORPEDO 2
#define AUDIO_INDEX_PHASER 3
#define AUDIO_INDEX_HIT 7
#define AUDIO_INDEX_BTS1 8
#define AUDIO_INDEX_BTS2 9
#define AUDIO_INDEX_BTS3 10
#define AUDIO_INDEX_BTS4 11
#define AUDIO_INDEX_BTS5 12
#define AUDIO_INDEX_BTS6 13
#define AUDIO_INDEX_POWER_DOWN 18
#define AUDIO_INDEX_POWER_CONTINUE 17
#define AUDIO_INDEX_POWER_UP 16
#define AUDIO_INDEX_WARP_DOWN 15
#define AUDIO_INDEX_WARP_UP 14
//#define PIN_IR_RECEIVER 11

static char* scAudioEffects[]={"KLAX.WAV", "P1MSG.WAV", "TORP1.WAV", "SPZER1.WAV", "BPD1.WAV",
"BPUP1.WAV", "DSTRT.WAV", "HULHIT.WAV", "BTS1.WAV", "BTS2.WAV", "BTS3.WAV",
"BT4.WAV", "BT5.WAV", "BT6.WAV", "WACC1.WAV", "WDCELL1.WAV", "PUP1A.WAV", "PUP1B.WAV", "PD1.WAV"};

//class ShipOperations;

class EN1701A
{
  public:
    volatile static boolean buttonInterrupt;
    volatile static boolean b_warp_mode_on;
    volatile static boolean b_red_alert_on;
    volatile static boolean b_phaser_on;
    volatile static boolean b_power_cycle;
    static byte sendByte;
    volatile static byte sbAudioIndex;
    volatile static unsigned long suiCurrentShipState;
    volatile static unsigned long suiPreviousShipState;
    static void svWriteShipState(bool set, byte pinset);
    //static ShipOperations mShipboardOperations;
};

#endif
