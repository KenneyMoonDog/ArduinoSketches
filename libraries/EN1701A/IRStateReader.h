#ifndef IRStateReader_h
#define IRStateReader_h

#include <IRLib.h>
#include <EN1701A.h>

class IRStateReader {
public:
  IRStateReader(int rp, unsigned int *currState, unsigned int *oldState);
  bool updateShipStateViaIR();
  bool cleanTimeouts(unsigned long timerMillis);
//  int getLatchDelay();
//  int getBrightness();

private:
  long lastDecodedValue = 0;
//  int latchDelay = 500;
//  int brightness = 0;

  unsigned int *pCurrentShipState;
  unsigned int *pOldShipState;

  void writeShipState(bool set, unsigned int pinset);
};

#endif
