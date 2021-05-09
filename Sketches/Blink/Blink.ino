/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */


int testPin1 = A1;
int testPin2 = A0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(testPin1, OUTPUT);
  pinMode(testPin2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(testPin1, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(testPin2, LOW);
  delay(1000);              // wait for a second
  digitalWrite(testPin1, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(testPin2, HIGH);
  delay(1000);              // wait for a second
}
