#include <Arduino.h>
#include "sensors.h"

const int sensorPin[5] = {
    10,  // OUT1
    11,  // OUT2
    12,  // OUT3
    13,  // OUT4
    14   // OUT5
};

int sensorValues[5];

void setupSensors()
{
    for (int i = 0; i < 5; i++)
    {
        pinMode(sensorPin[i], INPUT);
    }
}

void readSensors()
{
    for (int i = 0; i < 5; i++)
    {
        sensorValues[i] = digitalRead(sensorPin[i]);
    }
}

void printSensors()
{
    Serial.print("S1-S5: ");

    for (int i = 0; i < 5; i++)
    {
        Serial.print(sensorValues[i]);
        Serial.print(" ");
    }

    Serial.println();
}