#ifndef IRStateReader_h
#define IRStateReader_h

#include <IRLib.h>
#include <EN1701-REFIT.h>

class IRStateReader {
public:
  //IRStateReader(int rp, unsigned int *currState, unsigned int *oldState, byte *audioIndex);
  IRStateReader(int rp);
  bool updateShipStateViaIR();
  bool executeTimedOperations(unsigned long currentMillis);

private:
  long lastDecodedValue = 0;
};

#endif
