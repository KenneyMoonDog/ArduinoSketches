#include "Arduino.h"


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

class Nacells {

  #define NACELLS_POWER_ON 10
  #define NACELLS_POWER_OFF 20

  private:
  volatile byte nacellsPin;

  volatile unsigned long last_nacells_update_time = 0;
  volatile unsigned long nacells_delay_time = 5;
  volatile byte nacellsMode = NACELLS_POWER_ON;
  volatile float no_radians = 0;
  volatile int radianCounter = 0;
  volatile uint8_t nacellPowerLevel=0;
  //volatile byte impulseLevel = 0;

  Nacells();

  public:
    Nacells(byte PIN_IN) {
      nacellsPin = PIN_IN;
      pinMode(PIN_IN, OUTPUT);
      digitalWrite(nacellsPin, false);
    }

   NacellsUpdate(unsigned long current_nacells_update_time) {
     if (current_nacells_update_time > last_nacells_update_time + nacells_delay_time) {
        last_nacells_update_time = current_nacells_update_time;

        switch (nacellsMode) {
          case NACELLS_POWER_ON:
            if (no_radians < 3.13) {
              no_radians+=0.01;
            }
            else {
              no_radians=0;
            }
     
            nacellPowerLevel = round(sin(no_radians) * 190) + 64;
            analogWrite(nacellsPin, nacellPowerLevel);
            break;
          case NACELLS_POWER_OFF:
            break;
          default:
            break;
        }
     }
   }
};

class ImpulseEngine {

  #define SHUTDOWN 10
  #define STARTUP 20
  #define IDLE 30

  #define MAX_IMPULSE 225
  #define MAX_IDLE 150
  #define MIN_IDLE 50
  #define MIN_IMPULSE 0

  private:
  volatile byte impulsePin;

  volatile unsigned long last_impulse_update_time = 0;
  volatile unsigned long impulse_delay_time = 10;
  volatile byte impulseMode = IDLE;
  volatile byte impulseLevel = 0;

  volatile byte throttleUp = 0;

  ImpulseEngine();
  public: ImpulseEngine(byte PIN_IN) {
    impulsePin = PIN_IN;
    pinMode(PIN_IN, OUTPUT);
    digitalWrite(impulsePin, false);
  }

  public: virtual void ImpulseUpdate(unsigned long current_impulse_update_time){
    if (current_impulse_update_time > last_impulse_update_time + impulse_delay_time) {
      last_impulse_update_time = current_impulse_update_time;

      switch (impulseMode){
        case SHUTDOWN:
          if (impulseLevel > 0) {
            impulseLevel -= 10;

            if (impulseLevel < 0) {
              impulseLevel = 0;
            }
            analogWrite(impulsePin, impulseLevel );
          }
          break;
          
        case STARTUP:
          break;
        case IDLE:
          if (impulseLevel >= MAX_IDLE && throttleUp > 0) {
            throttleUp = -10;
          }
          else if (impulseLevel <= MIN_IDLE && throttleUp <=0) {
              throttleUp = 10;
          }
          
          impulseLevel += throttleUp;

          analogWrite(impulsePin, impulseLevel);
          break;

        default:
          break;
      }
    }
  }

  public: virtual void UpdateImpulseMode(byte impulseMode) {

  }
};

class ShuttleTerminal {

#define PROCESS_SPEED_PERIOD_1 50
#define PROCESS_SPEED_PERIOD_2 150
#define PROCESS_SPEED_PERIOD_3 250
#define PROCESS_SPEED_PERIOD_4 450

#define MODE_DURATION_1 1200
#define MODE_DURATION_2 2500
#define MODE_DURATION_3 4000
#define MODE_DURATION_4 8000

private:
  volatile byte terminalPin_A;
  volatile byte terminalPin_B;

  volatile unsigned long last_terminal_update_time = 0;
  volatile unsigned long terminal_delay_time = PROCESS_SPEED_PERIOD_3;

  volatile unsigned long last_mode_time = 0;
  volatile unsigned long mode_update_time = MODE_DURATION_2;

  ShuttleTerminal() {
  } 
  
  public: ShuttleTerminal(byte PIN_A, byte PIN_B) {
    terminalPin_A = PIN_A;
    terminalPin_B = PIN_B;

    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
  } 

  public: virtual void TerminalUpdate(unsigned long current_terminal_time){
    if (current_terminal_time > last_terminal_update_time + terminal_delay_time) {
      last_terminal_update_time = current_terminal_time;
      byte terminalState = random(0,4);
      digitalWrite(terminalPin_A, bitRead(terminalState,0));
      digitalWrite(terminalPin_B, bitRead(terminalState,1));
    }

    if (current_terminal_time > last_mode_time + mode_update_time) {
      last_mode_time = current_terminal_time;
      byte new_mode = random(0,4);
      
      switch (new_mode) {
        case 0:
          mode_update_time = MODE_DURATION_1;
          terminal_delay_time = PROCESS_SPEED_PERIOD_1;
          break;
        case 1:
          mode_update_time = MODE_DURATION_2;
          terminal_delay_time = PROCESS_SPEED_PERIOD_2;
          break;
        case 2:
          mode_update_time = MODE_DURATION_3;
          terminal_delay_time = PROCESS_SPEED_PERIOD_3;
          break;
        case 3:
          mode_update_time = MODE_DURATION_4;
          terminal_delay_time = PROCESS_SPEED_PERIOD_4;
          break;
        default:
          break;
      }
    }
  }

  public: virtual void setUpdateDelay( unsigned long newTerminalUpdateTime){
    terminal_delay_time = newTerminalUpdateTime;
  }
};

#define TERMINAL_1a 18
#define TERMINAL_1b 9
#define TERMINAL_2a 7
#define TERMINAL_2b 5
#define TERMINAL_3a 2
#define TERMINAL_3b 12 //iygu
#define TERMINAL_4a 3
#define TERMINAL_4b 19

#define FORWARD_LIGHTS 20
#define CABIN_LIGHTS 13
#define NACELLS 10 //ihuv
#define IMPULSE_ENGINE 11

ShuttleTerminal terminal_1(TERMINAL_1a,TERMINAL_1b);
ShuttleTerminal terminal_2(TERMINAL_2a,TERMINAL_2b);
ShuttleTerminal terminal_3(TERMINAL_3a,TERMINAL_3b);
ShuttleTerminal terminal_4(TERMINAL_4a,TERMINAL_4b);

ImpulseEngine impulseEngine(IMPULSE_ENGINE);
Nacells nacells(NACELLS);


byte testCount = 0;
unsigned long currentUpdateTime = 0;

void setup() {

  pinMode(FORWARD_LIGHTS, OUTPUT);
  pinMode(CABIN_LIGHTS, OUTPUT);


  digitalWrite(CABIN_LIGHTS, true);
  digitalWrite(FORWARD_LIGHTS, true);
}

void loop() {
  currentUpdateTime = millis();
  terminal_1.TerminalUpdate(currentUpdateTime);
  terminal_2.TerminalUpdate(currentUpdateTime);
  terminal_3.TerminalUpdate(currentUpdateTime);
  terminal_4.TerminalUpdate(currentUpdateTime);

  impulseEngine.ImpulseUpdate(currentUpdateTime);
  nacells.NacellsUpdate(currentUpdateTime);
}

void clearAll() {
  digitalWrite(TERMINAL_1a, false);
  digitalWrite(TERMINAL_1b, false);
  digitalWrite(TERMINAL_2a, false);
  digitalWrite(TERMINAL_2b, false);
  digitalWrite(TERMINAL_3a, false);
  digitalWrite(TERMINAL_3b, false);
  digitalWrite(TERMINAL_4a, false);
  digitalWrite(TERMINAL_4b, false);

  digitalWrite(FORWARD_LIGHTS, false);
  digitalWrite(NACELLS, false);
}