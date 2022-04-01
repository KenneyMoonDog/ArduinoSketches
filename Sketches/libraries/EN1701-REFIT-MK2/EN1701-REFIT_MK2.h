#ifndef EN1701-REFIT_MK2_h
#define EN1701-REFIT_MK2_h

#include "Arduino.h"
#include "SERIAL_COMM_MK2.h"

#define MODE_HELM 1
#define MODE_AUX_CONTROL 2
#define MODE_ENGINEERING 3

/*This set maps bits in an unsigned int to
an illuminated control section of the ship*/
/*#define SR_BUTTON_0 0 //byte 1
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
#define INCREASE_IMPULSE_ENGINES 21
#define DECREASE_IMPULSE_ENGINES 22
#define SWITCH_TO_WARP_MODE 23
#define SWITCH_TO_IMPULSE_MODE 24*/
//#define PHASER_OFF 21

#define AUDIO_INDEX_CANCEL 100

#define AUDIO_INDEX_RED_ALERT 0
#define AUDIO_INDEX_P1_MESSAGE 1
#define AUDIO_INDEX_TORPEDO 2
#define AUDIO_INDEX_PHASER 3
#define AUDIO_INDEX_DSTRT 6
#define AUDIO_INDEX_HIT 7
#define AUDIO_INDEX_BTS1 8
#define AUDIO_INDEX_BTS2 9
#define AUDIO_INDEX_BTS3 10
#define AUDIO_INDEX_BTS4 11
#define AUDIO_INDEX_BTS5 12
#define AUDIO_INDEX_BTS6 13
#define AUDIO_INDEX_POWER_DOWN 18

#define AUDIO_INDEX_WARP_DOWN 15
#define AUDIO_INDEX_WARP_UP 14
#define AUDIO_INDEX_LOUNGE 19

#define AUDIO_INDEX_MODE_T_LONG_TRANSPORT 21
#define AUDIO_INDEX_MODE_T_SHORT_TRANSPORT 22
#define AUDIO_INDEX_MODE_T_BUTTON 23

#define AUDIO_INDEX_POWER_UP 16    //"PUP1A.WAV"
#define AUDIO_INDEX_POWER_CONTINUE 17 //PUP1B.WAV
#define AUDIO_INDEX_MODE_T_BACKGROUND 20 //TBGND.WAV

#define AUDIO_INDEX_TLIFT_AUXCONTROL 24
#define AUDIO_INDEX_AUXCONTROL_CONTINUE 25
#define AUDIO_INDEX_TLIFT_BRIDGE 26
#define AUDIO_INDEX_TLIFT_ENGINEERING 27

//#define PIN_IR_RECEIVER 11

static char* scAudioEffects[]={(char*)"KLAX.WAV", (char*)"P1MSG.WAV", (char*)"TORP1.WAV", (char*)"SPZER1.WAV", (char*)"BPD1.WAV", (char*)"BPUP1.WAV", (char*)"DSTRT.WAV", (char*)"HULHIT.WAV", (char*)"BTS1.WAV", (char*)"BTS2.WAV", (char*)"BTS3.WAV", (char*)"BT4.WAV", (char*)"BT5.WAV", (char*)"BT6.WAV", (char*)"WACC1.WAV", (char*)"WDCELL1.WAV", (char*)"PUP1A.WAV", (char*)"PUP1B.WAV", (char*)"PD1.WAV", (char*)"LMIX.WAV", (char*)"TBGND.WAV", (char*)"TBBO.WAV", (char*)"TLBO.WAV", (char*)"TBTN.WAV", (char*)"TLAXA.WAV", (char*)"TLAXB.WAV", (char*)"TLBDG.WAV", (char*)"TLENG.WAV"};

static byte colorWhite[] = {10, 10, 10};
static byte colorAmber[] = {250, 69, 0};
static byte colorBlue[] = {0, 0, 255};
static byte colorOff[] = {0, 0, 0};

class EN1701A
{
  public:
    volatile static boolean buttonInterrupt;
    volatile static boolean b_warp_engine_on;
    volatile static boolean b_warp_mode_on;
    volatile static boolean b_red_alert_on;
    volatile static boolean b_phaser_on;
    volatile static boolean b_power_cycle;
    volatile static byte console_mode;
    volatile static boolean console_light_state;
    volatile static byte buttonPressed;

    static byte sendByte;
    volatile static byte sbAudioIndex;


    //volatile static unsigned long suiCurrentShipState;
    //volatile static unsigned long suiPreviousShipState;
    //static void svWriteShipState(bool set, byte pinset);
    //static ShipOperations mShipboardOperations;
};

#endif
