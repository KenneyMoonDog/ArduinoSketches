#ifndef IRStateReader_h
#define IRStateReader_h

#include <IRLib.h>
#include <EN1701A.h>

class IRStateReader {
public:
  IRStateReader(int rp, unsigned int *currState, unsigned int *oldState, byte *audioIndex);
  bool updateShipStateViaIR();
  bool executeTimedOperations(unsigned long currentMillis);

private:
  long lastDecodedValue = 0;
  unsigned int *pCurrentShipState;
  unsigned int *pOldShipState;
  byte *pAudioIndex;

  void writeShipState(bool set, unsigned int pinset);
};

#endif
