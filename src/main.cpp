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

enum Algorithm
{
    DFS,
    BFS
};

Algorithm selectedAlgorithm = DFS;

bool lastUp = HIGH;
bool lastDown = HIGH;
bool lastSelect = HIGH;

void drawMenu()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(25, 0);
    display.println("DSA MazeBot");

    display.setCursor(10, 20);

    if (selectedAlgorithm == DFS)
        display.println("> DFS");
    else
        display.println("  DFS");

    display.setCursor(10, 35);

    if (selectedAlgorithm == BFS)
        display.println("> BFS");
    else
        display.println("  BFS");

    display.setCursor(10, 52);
    display.println("SELECT = Start");

    display.display();
}

void showSelected()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(25, 10);
    display.println("Selected:");

    display.setTextSize(2);
    display.setCursor(40, 30);

    if (selectedAlgorithm == DFS)
        display.println("DFS");
    else
        display.println("BFS");

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

    Serial.println("DSA MazeBot menu ready");

    drawMenu();
}

void loop()
{
    bool currentUp = digitalRead(BUTTON_UP);
    bool currentDown = digitalRead(BUTTON_DOWN);
    bool currentSelect = digitalRead(BUTTON_SELECT);

    if (currentUp == LOW && lastUp == HIGH)
    {
        selectedAlgorithm = DFS;

        Serial.println("DFS highlighted");

        drawMenu();
    }

    if (currentDown == LOW && lastDown == HIGH)
    {
        selectedAlgorithm = BFS;

        Serial.println("BFS highlighted");

        drawMenu();
    }

    if (currentSelect == LOW && lastSelect == HIGH)
    {
        Serial.print("Selected algorithm: ");

        if (selectedAlgorithm == DFS)
            Serial.println("DFS");
        else
            Serial.println("BFS");

        showSelected();
    }

    lastUp = currentUp;
    lastDown = currentDown;
    lastSelect = currentSelect;

    delay(20);
}
