#ifndef EN1701A_h
#define EN1701A_h

#include "Arduino.h"

/*This set maps bits in an unsigned int to
an illuminated control section of the ship*/
#define SECTION_0 0
#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3
#define SECTION_4 4
#define SECTION_5 5
#define SECTION_6 6
#define SECTION_7 7
#define PRIMARY_SYSTEMS 8
#define RUNNING_LIGHTS 9
#define PHASER 10
#define TORPEDO 11
#define WARP_ENGINES 12
#define IMPULSE_ENGINES 13
#define POWER_CHANGE 14
//#define SHUTDOWN 15

/*This set maps PIN assignemnts to bit locations*/
#define PIN_SR_LATCH 15
#define PIN_SR_CLOCK 14
#define PIN_SR_SECTION_DATA 16
#define PIN_SR_SECTION_ENABLE 6
#define PIN_IR_RECEIVER 11
#define PIN_TORPEDO 7
#define PIN_PHASER 6
#define PIN_PRIMARY_SYSTEMS 4
#define PIN_RUNNING_LIGHTS 9
#define PIN_IMPULSE_DECK 11

#define STATE_POWER_OFF 0x000010

#endif
