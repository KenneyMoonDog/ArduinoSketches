#ifndef ButtonReader_MK2_h
#define ButtonReader_MK2_h

#define PIN_A_BUTTON 7
#define PIN_B_BUTTON 14
#define PIN_C_BUTTON 17
#define PIN_D_BUTTON 15

#define PIN_E_BUTTON 8
#define PIN_F_BUTTON 6
#define PIN_G_BUTTON 18
#define PIN_H_BUTTON 16

#define PIN_TEST_INTERRUPT 19

class ButtonReader_MK2 {
public:
  ButtonReader_MK2();
  void setupInterrupts();
  boolean pollButtons();

private:
  static void testInterruptPinFall();
  static boolean onButtonChangeDown(byte pinToTest, unsigned long &debounceTarget);
};
#endif
