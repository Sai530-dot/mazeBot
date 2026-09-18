#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "sensors.h"

/* ---------------- Buttons ---------------- */

#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_SELECT 6

/* ---------------- OLED ---------------- */

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

/* ---------------- Algorithms ---------------- */

enum Algorithm
{
    DFS,
    BFS,
    A_STAR
};

Algorithm selectedAlgorithm = DFS;

/* ---------------- Robot States ---------------- */

enum RobotState
{
    MENU,
    RUNNING,
    FINISHED
};

RobotState robotState = MENU;

/* ---------------- Button States ---------------- */

bool lastUp = HIGH;
bool lastDown = HIGH;
bool lastSelect = HIGH;

/* ---------------- OLED Functions ---------------- */

void drawMenu()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(25, 0);
    display.println("DSA MazeBot");

    /* DFS */
    display.setCursor(10, 16);

    if (selectedAlgorithm == DFS)
        display.println("> DFS");
    else
        display.println("  DFS");

    /* BFS */
    display.setCursor(10, 28);

    if (selectedAlgorithm == BFS)
        display.println("> BFS");
    else
        display.println("  BFS");

    /* A* */
    display.setCursor(10, 40);

    if (selectedAlgorithm == A_STAR)
        display.println("> A*");
    else
        display.println("  A*");

    display.setCursor(10, 54);
    display.println("SELECT = Start");

    display.display();
}


void showRunning()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(38, 5);
    display.println("Running...");

    display.setCursor(25, 25);
    display.println("Algorithm:");

    display.setTextSize(2);
    display.setCursor(42, 40);

    if (selectedAlgorithm == DFS)
        display.println("DFS");
    else if (selectedAlgorithm == BFS)
        display.println("BFS");
    else if (selectedAlgorithm == A_STAR)
        display.println("A*");

    display.display();
}


void showFinished()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(35, 10);
    display.println("Maze Done!");

    display.setCursor(20, 30);
    display.println("SELECT = Menu");

    display.display();
}

/* ---------------- Menu ---------------- */

void handleMenu()
{
    bool currentUp = digitalRead(BUTTON_UP);
    bool currentDown = digitalRead(BUTTON_DOWN);
    bool currentSelect = digitalRead(BUTTON_SELECT);

    /* UP button */
    if (currentUp == LOW && lastUp == HIGH)
    {
        if (selectedAlgorithm == DFS)
            selectedAlgorithm = A_STAR;
        else if (selectedAlgorithm == BFS)
            selectedAlgorithm = DFS;
        else if (selectedAlgorithm == A_STAR)
            selectedAlgorithm = BFS;

        Serial.println("Moved UP");

        drawMenu();
    }

    /* DOWN button */
    if (currentDown == LOW && lastDown == HIGH)
    {
        if (selectedAlgorithm == DFS)
            selectedAlgorithm = BFS;
        else if (selectedAlgorithm == BFS)
            selectedAlgorithm = A_STAR;
        else if (selectedAlgorithm == A_STAR)
            selectedAlgorithm = DFS;

        Serial.println("Moved DOWN");

        drawMenu();
    }

    /* SELECT button */
    if (currentSelect == LOW && lastSelect == HIGH)
    {
        Serial.print("Starting ");

        if (selectedAlgorithm == DFS)
            Serial.println("DFS...");
        else if (selectedAlgorithm == BFS)
            Serial.println("BFS...");
        else if (selectedAlgorithm == A_STAR)
            Serial.println("A*...");

        robotState = RUNNING;

        showRunning();
    }

    lastUp = currentUp;
    lastDown = currentDown;
    lastSelect = currentSelect;
}

/* ---------------- Running ---------------- */

void runRobot()
{
    static unsigned long lastSensorPrint = 0;

    if (millis() - lastSensorPrint >= 200)
    {
        readSensors();
        printSensors();

        lastSensorPrint = millis();
    }

    bool currentSelect = digitalRead(BUTTON_SELECT);

    if (currentSelect == LOW && lastSelect == HIGH)
    {
        Serial.println("Stopping robot");

        robotState = FINISHED;

        showFinished();
    }

    lastSelect = currentSelect;
}

/* ---------------- Finished ---------------- */

void handleFinished()
{
    bool currentSelect = digitalRead(BUTTON_SELECT);

    /*
     * Press SELECT to return to menu.
     */
    if (currentSelect == LOW && lastSelect == HIGH)
    {
        Serial.println("Returning to menu");

        robotState = MENU;

        drawMenu();
    }

    lastSelect = currentSelect;
}

/* ---------------- Setup ---------------- */

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);

    setupSensors();

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("OLED initialization failed!");

        while (true)
        {
        }
    }

    Serial.println("DSA MazeBot ready");

    drawMenu();
}

/* ---------------- Main Loop ---------------- */

void loop()
{
    if (robotState == MENU)
    {
        handleMenu();
    }
    else if (robotState == RUNNING)
    {
        runRobot();
    }
    else if (robotState == FINISHED)
    {
        handleFinished();
    }

    delay(20);
}