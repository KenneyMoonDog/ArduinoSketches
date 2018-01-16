#ifndef ButtonReader_h
#define ButtonReader_h

class ButtonReader {
public:
  ButtonReader();
  void setupInterrupts();
  boolean scanButtons();

private:
  static void interruptTest();
  static void torpedo_button_interrupt();
  boolean phaser_button_change();
  boolean torpedo_button_change();
  //static ShipOperations *pShipOperations;
};
#endif
