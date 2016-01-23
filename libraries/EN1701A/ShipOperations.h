#ifndef ShipOperations_h
#define ShipOperations_h

#include <EN1701A.h>
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

class ShipOperations {
public:
   ShipOperations( unsigned int *currState, unsigned int *oldState );
   void ApplyShipLogic();
   void cleanTimeouts();
   //void ApplyLights();
   //void ApplySounds();
   void clearAll();

 private:
   SdReader card;    // This object holds the information for the card
   FatVolume vol;    // This holds the information for the partition on the card
   FatReader root;   // This holds the information for the filesystem on the card
   FatReader f;      // This holds the information for the file we're play

   WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

   unsigned int *pCurrentShipState;
   unsigned int *pOldShipState;
   unsigned int mCurrentPinState = 0;
   unsigned int mPreviousPinState = 0;
   byte mSectionData = 0; //a byte with each bit represents a different lighting section

   void updateSectionDataRegister();
   void sdErrorCheck(void);
   int freeRam(void);
   void setupSound();
   void playcomplete(char *name);
   void playfile(char *name);
   void writeShipState(bool set, unsigned int pinset);
   bool readCurrentShipState(unsigned int pinset);
   bool readOldShipState(unsigned int pinset);
};
#endif
