#ifndef ButtonReader_h
#define ButtonReader_h

class ButtonReader {
public:
  ButtonReader();
  void setupInterrupts();
  boolean pollButtons();

private:
  static void testInterruptPinFall();
  static void updateShipState_fromInterrupt(byte pinToTest);
  static boolean onButtonChangeDown(byte pinToTest, unsigned long &debounceTarget);
};
#endif
