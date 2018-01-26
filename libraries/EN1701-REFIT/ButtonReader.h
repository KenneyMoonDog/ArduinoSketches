#ifndef ButtonReader_h
#define ButtonReader_h

class ButtonReader {
public:
  ButtonReader();
  void setupInterrupts();
  boolean pollButtons();

private:
  static void testInterruptPin();
  static void updateShipState_fromInterrupt(byte pinToTest, boolean newState);
  void updateShipState_fromPolledButton(byte pinToTest, boolean newState);
  static boolean onButtonChange(byte pinToTest, boolean &stateTarget, unsigned long &debounceTarget, boolean bDownOnly );
};
#endif
