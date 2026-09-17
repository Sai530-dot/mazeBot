#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 8
#define SCL_PIN 9

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting OLED...");

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("OLED initialization FAILED!");

        while (true)
        {
        }
    }

    Serial.println("OLED connected!");

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(20, 15);
    display.println("DSA MazeBot");

    display.setCursor(30, 35);
    display.println("OLED OK!");

    display.display();
}

void loop()
{
}
