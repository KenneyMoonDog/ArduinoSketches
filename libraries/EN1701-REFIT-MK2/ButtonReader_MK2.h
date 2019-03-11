#ifndef ButtonReader_MK2_h
#define ButtonReader_MK2_h

class ButtonReader_MK2 {
public:
  ButtonReader_MK2();
  void setupInterrupts();
  boolean pollButtons();

private:
  static void setMode(byte mode);
  static void testInterruptPinFall();
  static void updateShipState_fromInterrupt(byte pinToTest);
  static boolean onButtonChangeDown(byte pinToTest, unsigned long &debounceTarget);
};
#endif
