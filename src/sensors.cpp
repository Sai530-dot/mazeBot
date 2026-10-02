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
    // 12-bit ADC:
    // readings range from 0 to 4095
    analogReadResolution(12);

    for (int i = 0; i < 5; i++)
    {
        pinMode(sensorPin[i], INPUT);
    }
}

void readSensors()
{
    for (int i = 0; i < 5; i++)
    {
        sensorValues[i] = analogRead(sensorPin[i]);
    }
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
        {
            Serial.print(" | ");
        }
    }

    Serial.println();
}