#ifndef ShipOperations_h
#define ShipOperations_h

#include <EN1701-REFIT.h>
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

class ShipOperations {
public:
   ShipOperations();
   void ApplyShipLogic();
   void audioCheck();
   void clearAll();

 private:
   SdReader card;    // This object holds the information for the card
   FatVolume vol;    // This holds the information for the partition on the card
   FatReader root;   // This holds the information for the filesystem on the card
   FatReader f;      // This holds the information for the file we're play
   char* pCurrentFilePlaying = NULL;

   WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

   void updateSection_DataRegister();

   //void sdErrorCheck(void);
   int freeRam(void);
   void setupSound();
   void playFile();
   void stopPlaying();
   bool readCurrentShipState(byte pinset);
   bool readOldShipState(byte pinset);
};
#endif
