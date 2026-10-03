#include <Arduino.h>
#include "motors.h"

/* -------- TB6612 Pins -------- */

#define PWMA 39
#define AIN2 16
#define AIN1 15
#define STBY 38

#define BIN1 17
#define BIN2 18
#define PWMB 40

/* PIN SETUP */

void setupMotors()
{
    pinMode(PWMA, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);

    pinMode(PWMB, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);

    pinMode(STBY, OUTPUT);

    /*
     * SAFE START:
     * driver disabled and motors stopped.
     */
    digitalWrite(STBY, LOW);

    digitalWrite(PWMA, LOW);
    digitalWrite(PWMB, LOW);

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);

    Serial.println("Motors initialized");
}

/* MOVE FORWARD */

void moveForward()
{
    digitalWrite(STBY, HIGH);

    // Motor A
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    // Motor B
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    // Full speed for initial test
    digitalWrite(PWMA, HIGH);
    digitalWrite(PWMB, HIGH);
}

/* MOVE BACKWARD */

void moveBackward()
{
    digitalWrite(STBY, HIGH);

    // Motor A
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    // Motor B
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    digitalWrite(PWMA, HIGH);
    digitalWrite(PWMB, HIGH);
}

/* LEFT WHEEL */

void turnLeft()
{
    digitalWrite(STBY, HIGH);

    // Motor A backward
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    // Motor B forward
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    digitalWrite(PWMA, HIGH);
    digitalWrite(PWMB, HIGH);
}

/* RIGHT WHEEL */

void turnRight()
{
    digitalWrite(STBY, HIGH);

    // Motor A forward
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    // Motor B backward
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    digitalWrite(PWMA, HIGH);
    digitalWrite(PWMB, HIGH);
}

/* STOP MOTORS */

void stopMotors()
{
    digitalWrite(PWMA, LOW);
    digitalWrite(PWMB, LOW);

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);

    digitalWrite(STBY, LOW);
}