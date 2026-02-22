#include "TB6612FNG_XCR.h"

TB6612FNG_XCR leftMotor;
TB6612FNG_XCR rightMotor;

#define STBY_PIN 7

#define LS 2
#define CS 3
#define RS 4

void setup() {
    leftMotor.attach(5, A1, A2, "Left");
    rightMotor.attach(6, A3, A4, "Right");

    leftMotor.setStandbyPin(STBY_PIN);
    rightMotor.setStandbyPin(STBY_PIN);

    rightMotor.invert(true);

    pinMode(LS, INPUT);
    pinMode(CS, INPUT);
    pinMode(RS, INPUT);
}

void loop() {
    int l = digitalRead(LS);
    int c = digitalRead(CS);
    int r = digitalRead(RS);

    if (c == 1 && l == 0 && r == 0) {
        forward();
    } else if (l == 1) {
        turnLeft();
    } else if (r == 1) {
        turnRight();
    } else {
        stopMotors();
    }
}

void forward() {
    leftMotor.write(180);
    rightMotor.write(180);
}

void turnLeft() {
    leftMotor.write(100);
    rightMotor.write(180);
}

void turnRight() {
    leftMotor.write(180);
    rightMotor.write(100);
}

void stopMotors() {
    leftMotor.stop();
    rightMotor.stop();
}
