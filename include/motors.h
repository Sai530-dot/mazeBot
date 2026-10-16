#ifndef MOTORS_H
#define MOTORS_H

void setupMotors();
void driveMotors(int leftSpeed, int rightSpeed);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();

#endif