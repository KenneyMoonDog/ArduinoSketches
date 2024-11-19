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
  volatile unsigned long nacells_delay_time =3;
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
            if (no_radians < TWO_PI) {
              no_radians+=0.01;
            }
            else {
              no_radians=0;
            }
     
            nacellPowerLevel = sin(no_radians) * 95 + 159;
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

class ImpulseEngine2 {

  #define SHUTDOWN 10
  #define STARTUP 20
  #define IDLE 30

  #define MAX_IMPULSE 254
  #define MAX_IDLE 150
  #define MIN_IDLE 25
  #define MIN_IMPULSE 5

  private:
  volatile byte impulsePin;

  volatile unsigned long last_impulse_update_time = 0;
  volatile unsigned long impulse_delay_time = 10;

  volatile unsigned long last_flash_update_time = 0;

  volatile unsigned long last_idle_mode_update_time = 0;
  volatile unsigned long idle_duration = 20000;

  volatile unsigned long last_shutdown_update_time = 0;
  volatile unsigned long shutdown_duration = 7500;

  volatile int flash_delay = 75;
  volatile int flash_duration = 10;
  volatile byte impulseMode = STARTUP;
  volatile byte impulseLevel = 0;

  ImpulseEngine2();

  public: ImpulseEngine2(byte PIN_IN) {
    impulsePin = PIN_IN;
    pinMode(PIN_IN, OUTPUT);
    digitalWrite(impulsePin, false);
  }

  public: virtual void ImpulseUpdate(unsigned long current_impulse_update_time){

    static float impulse_no_radians = asin(MAX_IMPULSE/190);
    static int impulse_radianCounter = 0;

    static byte flashCount = 0;
    static bool flashOn = false;
    static bool idleOn = false;
    static bool initialIdle = true;
    static bool initialShutdown = true;
    static uint8_t ramp_up_impulse_level = 0;

    byte flashMax = 20;
    int flash_delay_time = 0;
    uint8_t flash_level_increment = MAX_IMPULSE/flashMax;
    uint8_t shutdown_increment = 1;

    if (current_impulse_update_time > last_impulse_update_time + impulse_delay_time) {
      last_impulse_update_time = current_impulse_update_time;

      switch (impulseMode){

        case SHUTDOWN:
          if (initialShutdown) {
            last_shutdown_update_time = current_impulse_update_time;

            if (impulseLevel < (MAX_IMPULSE - shutdown_increment)){
              impulseLevel += shutdown_increment;
            }
            else {
              initialShutdown = false;
            }
          }
          else {
            if (impulseLevel >= shutdown_increment) {
              impulseLevel -= shutdown_increment;
            }
            else {
              impulseLevel = 0;
            }
          }

          if (current_impulse_update_time > last_shutdown_update_time + shutdown_duration) {
              initialShutdown = true;
              last_shutdown_update_time = 0;
              impulseMode = STARTUP;
          }

          break;
          
        case STARTUP:
          if (flashOn) {
            flash_delay_time = flash_duration;
          }
          else {
            flash_delay_time = flash_delay;
          }

          if ( current_impulse_update_time > last_flash_update_time + flash_delay_time) {
            last_flash_update_time = current_impulse_update_time;
            if (flashCount++ <= flashMax){
              if (!flashOn) {
                impulseLevel = ramp_up_impulse_level;
                flashOn = true;
              }
              else{
                impulseLevel = 0;
                ramp_up_impulse_level += flash_level_increment;
                flashOn = false;
              }
            }
            else {
              impulseMode = IDLE;
              last_flash_update_time = 0;
              flashOn = false;
              flashCount = 0;
            }
          } 
          break;

        case IDLE:

          if (initialIdle) {
            last_idle_mode_update_time = current_impulse_update_time;
            initialIdle = false;
          }

          if (impulse_no_radians < TWO_PI) {
            impulse_no_radians+=0.02;
          }
          else {
            impulse_no_radians=0;
          }
          impulseLevel = MAX_IDLE + sin(impulse_no_radians) * (MAX_IMPULSE - MAX_IDLE); //oscillate around MAX_IDLE
          
          if (current_impulse_update_time > last_idle_mode_update_time + idle_duration) {
            initialIdle = true;
            last_idle_mode_update_time = 0;
            impulseMode = SHUTDOWN;
          }
          break;

        default:
          break;
      } //end switch
      analogWrite(impulsePin, impulseLevel);
    } //end timing
  }
};

class ImpulseEngine {

  #define SHUTDOWN 10
  #define STARTUP 20
  #define IDLE 30

  #define MAX_IMPULSE 225
  #define MAX_IDLE 100
  #define MIN_IDLE 25
  #define MIN_IMPULSE 5

  private:
  volatile byte impulsePin;

  volatile unsigned long last_impulse_update_time = 0;
  volatile unsigned long impulse_delay_time = 10;
  volatile byte impulseMode = STARTUP;
  volatile byte impulseLevel = 0;
  volatile byte throttleUp = 0;

  ImpulseEngine();

  public: ImpulseEngine(byte PIN_IN) {
    impulsePin = PIN_IN;
    pinMode(PIN_IN, OUTPUT);
    digitalWrite(impulsePin, false);
  }

  public: virtual void ImpulseUpdate(unsigned long current_impulse_update_time){

    static bool pulses[] = {true, false, false, false, false, false, false, false};
    static byte pulseCount = 0;

    static float impulse_no_radians = asin(MAX_IMPULSE/190);
    static int impulse_radianCounter = 0;

    static unsigned long flicker_delay_time = 3000;
    static byte flickerCount = 0;
    static bool flicker_off = false;

    if (current_impulse_update_time > last_impulse_update_time + impulse_delay_time) {
      last_impulse_update_time = current_impulse_update_time;

      switch (impulseMode){
        case SHUTDOWN:
          break;
          
        case STARTUP:
          switch (pulseCount) {
            case 0: 
            case 1:
            case 2:  
            case 3:
              if (pulses[pulseCount]) {  //flash
                if (impulseLevel <= MAX_IMPULSE) {
                  impulseLevel += 40;
                }
                else {
                  pulses[pulseCount] = false;
                }
              }
              else { //the flashing is over.. reset the impulse level to 0
                if (impulseLevel > 0){
                  impulseLevel = 0;
                }
                else {
                  pulses[++pulseCount] = true;
                }
              }
              break;
            
            case 4:
              if (pulses[pulseCount]) {  //flash
                if (impulseLevel <= MAX_IMPULSE) {
                  impulseLevel += 40;
                }
                else {  //the flash is over.. leave impulse level at MAX_IMPULSE
                  pulses[pulseCount] = false;
                }
              }
              else { 
                pulses[++pulseCount] = true;
              }
              break;        
              
            case 5: //at pulse five we ramp down to impulse level 0.. remain there
              if (pulses[pulseCount]) {
                if (impulseLevel > 0) {
                  if (impulse_no_radians < TWO_PI) {
                    impulse_no_radians+=0.01;
                  }
                  else {
                    impulse_no_radians = 0;
                  }
                  impulseLevel = sin(impulse_no_radians) * MAX_IMPULSE;
                }
                else {
                  pulses[pulseCount] = false;
                }
              } 
              else {
                pulses[++pulseCount] = true;
              }  
              break;

            case 6: //increase gradually to MAX_IDLE
              if (pulses[pulseCount]) {
                if (impulseLevel < MAX_IDLE){
                  if (impulse_no_radians < TWO_PI) {
                    impulse_no_radians+=0.04;
                  }
                  else {
                    impulse_no_radians = 0;
                  }
                  impulseLevel = 1 + sin(impulse_no_radians) * MAX_IDLE;
                }
                else {
                  pulses[pulseCount] = false;
                }   
              } 
              else { //move to the next phase
                pulses[++pulseCount] = true;
              }
              break;    

            case 7: //we are now in idle state
              impulseMode = IDLE;
              break;
          } //end pulse switch

          analogWrite(impulsePin, impulseLevel);
          break;

        case IDLE:
          if (!flicker_off){
            flicker_delay_time -= impulse_delay_time;
          }

          if (flicker_delay_time > 0){
            if (impulse_no_radians < TWO_PI) {
              impulse_no_radians+=0.07;
            }
            else {
              impulse_no_radians=0;
            }
            impulseLevel = MAX_IDLE + sin(impulse_no_radians) * 30; //oscillate up to 30 levels around MAX_IDLE
          }
          else {   //flicker delay time has gone to 0         
            if (flickerCount++ < 1){
              analogWrite(impulsePin, 0);
              flicker_off = true;
            }
            else {
              if (flicker_off) {
                flicker_off = false;
                flicker_delay_time = 1000 * random(2,6);
                flickerCount = 0;
              }
            }
          }
          
          if (!flicker_off) {
            analogWrite(impulsePin, impulseLevel);
          }
          break;

        default:
          break;
      }
    }
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
#define TERMINAL_3b 12 
#define TERMINAL_4a 3
#define TERMINAL_4b 19

#define FORWARD_LIGHTS 20
#define CABIN_LIGHTS 13
#define NACELLS 10 
#define IMPULSE_ENGINE 11

ShuttleTerminal terminal_1(TERMINAL_1a,TERMINAL_1b);
ShuttleTerminal terminal_2(TERMINAL_2a,TERMINAL_2b);
ShuttleTerminal terminal_3(TERMINAL_3a,TERMINAL_3b);
ShuttleTerminal terminal_4(TERMINAL_4a,TERMINAL_4b);

ImpulseEngine2 impulseEngine(IMPULSE_ENGINE);
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
