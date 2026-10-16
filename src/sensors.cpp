

#include <Arduino.h>
#include "sensors.h"

// Same five digital sensor pins as your earlier project.
const int sensorPin[5] = {10, 11, 12, 13, 14};
int sensorValues[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};

void setupSensors()
{
    for (int i = 0; i < 5; i++)
        pinMode(sensorPin[i], INPUT);
}

void readSensors()
{
    for (int i = 0; i < 5; i++)
        sensorValues[i] = digitalRead(sensorPin[i]);
}

void printSensors()
{
    for (int i = 0; i < 5; i++)
    {
        Serial.print("S");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(sensorValues[i]);
        if (i < 4)
            Serial.print(" | ");
    }
    Serial.println();
}