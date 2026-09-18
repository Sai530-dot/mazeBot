#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

extern int sensorValues[5];

void setupSensors();
void readSensors();
void printSensors();

#endif