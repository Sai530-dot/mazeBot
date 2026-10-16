

#include <Arduino.h>
#include "motors.h"

// Existing TB6612FNG control wiring.
#define PWMA 39
#define AIN1 15
#define AIN2 16
#define STBY 38
#define BIN1 17
#define BIN2 18
#define PWMB 40

// "Left" and "right" mean facing FORWARD toward your IR sensor.
// Set this to false if AO1/AO2 actually power the right wheel.
const bool LEFT_MOTOR_IS_A = true;

// If a wheel spins backward during the motor test, change its flag.
const bool MOTOR_A_REVERSED = false;
const bool MOTOR_B_REVERSED = true;

const int TEST_SPEED = 140; // PWM: 0-255

static void setMotor(int speed, int pwmPin, int in1, int in2, bool reversed)
{
    speed = constrain(speed, -255, 255);

    if (reversed)
        speed = -speed;

    if (speed == 0)
    {
        analogWrite(pwmPin, 0);
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        return;
    }

    digitalWrite(in1, speed > 0 ? HIGH : LOW);
    digitalWrite(in2, speed > 0 ? LOW : HIGH);
    analogWrite(pwmPin, abs(speed));
}

void setupMotors()
{
    pinMode(PWMA, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(STBY, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);

    stopMotors(); // Never start the wheels during boot.
}

// Signed speeds: +forward, -backward, 0=stop.
void driveMotors(int leftSpeed, int rightSpeed)
{
    if (leftSpeed == 0 && rightSpeed == 0)
    {
        stopMotors();
        return;
    }

    int aSpeed = LEFT_MOTOR_IS_A ? leftSpeed : rightSpeed;
    int bSpeed = LEFT_MOTOR_IS_A ? rightSpeed : leftSpeed;

    digitalWrite(STBY, HIGH);
    setMotor(aSpeed, PWMA, AIN1, AIN2, MOTOR_A_REVERSED);
    setMotor(bSpeed, PWMB, BIN1, BIN2, MOTOR_B_REVERSED);
}

void moveForward() { driveMotors(TEST_SPEED, TEST_SPEED); }
void moveBackward() { driveMotors(-TEST_SPEED, -TEST_SPEED); }
void turnLeft() { driveMotors(-TEST_SPEED, TEST_SPEED); }
void turnRight() { driveMotors(TEST_SPEED, -TEST_SPEED); }

void stopMotors()
{
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    digitalWrite(STBY, LOW);
}