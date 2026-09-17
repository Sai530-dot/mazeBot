#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_SELECT 6

#define SDA_PIN 8
#define SCL_PIN 9

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);

bool lastUp = HIGH;
bool lastDown = HIGH;
bool lastSelect = HIGH;

void showMessage(const char *message)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(20, 15);
    display.println("DSA MazeBot");

    display.setCursor(20, 35);
    display.println(message);

    display.display();
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("OLED initialization failed!");

        while (true)
        {
        }
    }

    showMessage("Test buttons");

    Serial.println("Button test ready");
}

void loop()
{
    bool currentUp = digitalRead(BUTTON_UP);
    bool currentDown = digitalRead(BUTTON_DOWN);
    bool currentSelect = digitalRead(BUTTON_SELECT);

    if (currentUp == LOW && lastUp == HIGH)
    {
        Serial.println("UP pressed");
        showMessage("UP pressed!");
    }

    if (currentDown == LOW && lastDown == HIGH)
    {
        Serial.println("DOWN pressed");
        showMessage("DOWN pressed!");
    }

    if (currentSelect == LOW && lastSelect == HIGH)
    {
        Serial.println("SELECT pressed");
        showMessage("SELECT pressed!");
    }

    lastUp = currentUp;
    lastDown = currentDown;
    lastSelect = currentSelect;

    delay(20);
}
