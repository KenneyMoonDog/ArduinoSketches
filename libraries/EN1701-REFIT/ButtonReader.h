#ifndef ButtonReader_h
#define ButtonReader_h

class ButtonReader {
public:
  ButtonReader();
  void setupInterrupts();


private:
  //static void checkTorpedoPin();
  //static void checkPhaserPin();
  static void testInterruptPin();
  static void updateShipState(byte pinToTest, boolean newState);
  static boolean onButtonChange(byte pinToTest, boolean &stateTarget, unsigned long &debounceTarget, boolean bDownOnly );
  static boolean power_button_change();
  static boolean phaser_button_change();
  static boolean torpedo_button_change();
  //static ShipOperations *pShipOperations;
};
#endif
