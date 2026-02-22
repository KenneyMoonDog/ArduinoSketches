#include "TB6612FNG_XCR.h"

TB6612FNG_XCR leftMotor;
TB6612FNG_XCR rightMotor;

#define STBY_PIN 7

#define EDGE_LEFT 2
#define EDGE_RIGHT 3
#define ENEMY_SENSOR 4

void setup() {
    leftMotor.attach(5, A1, A2, "Left");
    rightMotor.attach(6, A3, A4, "Right");

    leftMotor.setStandbyPin(STBY_PIN);
    rightMotor.setStandbyPin(STBY_PIN);

    rightMotor.invert(true);

    pinMode(EDGE_LEFT, INPUT);
    pinMode(EDGE_RIGHT, INPUT);
    pinMode(ENEMY_SENSOR, INPUT);

    leftMotor.enableSumoMode(&leftMotor, &rightMotor);
    rightMotor.enableSumoMode(&leftMotor, &rightMotor);
}

void loop() {
    bool edgeL = digitalRead(EDGE_LEFT);
    bool edgeR = digitalRead(EDGE_RIGHT);
    bool enemy = digitalRead(ENEMY_SENSOR);

    if (edgeL == 1 && edgeR == 0) {
        sumoReverseRight();
    } else if (edgeR == 1 && edgeL == 0) {
        sumoReverseLeft();
    } else if (edgeL == 1 && edgeR == 1) {
        sumoReverseBack();
    } else if (enemy == 1) {
        sumoAttack();
    } else {
        sumoSearch();
    }
}

void sumoReverseRight() {
    leftMotor.sumoControl(-180, -100);
    delay(400);
    leftMotor.sumoControl(180, 100);
    delay(300);
}

void sumoReverseLeft() {
    leftMotor.sumoControl(-100, -180);
    delay(400);
    leftMotor.sumoControl(100, 180);
    delay(300);
}

void sumoReverseBack() {
    leftMotor.sumoControl(-180, -180);
    delay(500);
}

void sumoAttack() {
    leftMotor.sumoControl(255, 255);
}

void sumoSearch() {
    leftMotor.sumoControl(180, 120);
}
