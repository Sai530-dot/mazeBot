#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "sensors.h"
#include "motors.h"

// Existing ESP32-S3 wiring.
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_SELECT 6
#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Same digital detection polarity as your original code.
const int BLACK_LEVEL = LOW;

// Conservative initial line-following speeds (PWM 0-255).
const int BASE_SPEED = 135;
const int MAX_SPEED = 190;
const int TURN_GAIN = 45;

// S1 should be the robot's LEFT sensor when facing the IR sensor.
// If your board is mounted in reverse, set this to true.
const bool REVERSE_SENSOR_ORDER = false;

const char *menuItems[] = {
    "DFS (line)",
    "BFS (line)",
    "A* (line)",
    "Motor test",
    "Sensor test"};
const int MENU_COUNT = 5;
int selectedItem = 0;

// DFS / BFS / A* are labels for now; all use basic line following.
enum RobotMode
{
    MENU,
    SENSORS,
    MOTOR_TEST,
    FOLLOWING,
    STOPPED
};
RobotMode mode = MENU;

bool lastUp = HIGH;
bool lastDown = HIGH;
bool lastSelect = HIGH;
unsigned long lastButtonEvent = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastSerialPrint = 0;

// Brief motor bench test. Speeds are LEFT, RIGHT (robot perspective).
struct TestStep
{
    const char *name;
    int leftPWM;
    int rightPWM;
    unsigned long durationMs;
};

const TestStep motorSteps[] = {
    {"Ready", 0, 0, 500},
    {"Left motor", 140, 0, 700},
    {"Pause", 0, 0, 400},
    {"Right motor", 0, 140, 700},
    {"Pause", 0, 0, 400},
    {"Both motors", 140, 140, 800}};
const int TEST_STEP_COUNT = sizeof(motorSteps) / sizeof(motorSteps[0]);
int testStep = 0;
unsigned long testStepStarted = 0;

void drawMenu()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 0);
    display.println("DSA MazeBot");

    // Four visible menu entries; scroll to reach the fifth.
    int first = (selectedItem < 4) ? 0 : 1;
    for (int i = first; i < first + 4; i++)
    {
        display.setCursor(5, 13 + (i - first) * 11);
        display.print(i == selectedItem ? "> " : "  ");
        display.println(menuItems[i]);
    }

    display.setCursor(8, 57);
    display.print("SELECT = Start");
    display.display();
}

void drawSensorScreen()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 0);
    display.println("IR SENSOR TEST");
    display.setCursor(0, 14);
    display.println("S1 S2 S3 S4 S5");
    display.setCursor(0, 27);
    for (int i = 0; i < 5; i++)
    {
        display.print(sensorValues[i]);
        if (i < 4)
            display.print("  ");
    }
    display.setCursor(0, 40);
    display.print("BLACK: ");
    for (int i = 0; i < 5; i++)
        display.print(sensorValues[i] == BLACK_LEVEL ? 'B' : '-');
    display.setCursor(0, 55);
    display.println("SELECT = Back");
    display.display();
}

void drawStatus(const char *heading, const char *detail)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 5);
    display.println(heading);
    display.setCursor(5, 27);
    display.println(detail);
    display.setCursor(5, 54);
    display.println("SELECT = Stop/Menu");
    display.display();
}

void stopAndShow(const char *message)
{
    stopMotors();
    mode = STOPPED;
    drawStatus("Motors stopped", message);
    Serial.println(message);
}

void startMotorTestStep()
{
    testStepStarted = millis();
    driveMotors(motorSteps[testStep].leftPWM,
                motorSteps[testStep].rightPWM);
    drawStatus("MOTOR BENCH TEST", motorSteps[testStep].name);
    Serial.print("Motor test: ");
    Serial.println(motorSteps[testStep].name);
}

void startSelectedMode()
{
    if (selectedItem == 4)
    {
        mode = SENSORS;
        readSensors();
        drawSensorScreen();
    }
    else if (selectedItem == 3)
    {
        mode = MOTOR_TEST;
        testStep = 0;
        startMotorTestStep();
    }
    else
    {
        mode = FOLLOWING;
        drawStatus("LINE FOLLOWING", menuItems[selectedItem]);
        Serial.println("Line following (not DFS/BFS/A* yet)");
    }
}

void handleButtons()
{
    bool up = digitalRead(BUTTON_UP);
    bool down = digitalRead(BUTTON_DOWN);
    bool select = digitalRead(BUTTON_SELECT);

    bool upPressed = (up == LOW && lastUp == HIGH);
    bool downPressed = (down == LOW && lastDown == HIGH);
    bool selectPressed = (select == LOW && lastSelect == HIGH);

    lastUp = up;
    lastDown = down;
    lastSelect = select;

    if (millis() - lastButtonEvent < 150)
        return; // Simple button debounce.

    if (selectPressed)
    {
        lastButtonEvent = millis();
        if (mode == MENU)
            startSelectedMode();
        else if (mode == SENSORS || mode == STOPPED)
        {
            stopMotors();
            mode = MENU;
            drawMenu();
        }
        else
            stopAndShow("Press SELECT for menu");
        return;
    }

    if (mode == MENU && (upPressed || downPressed))
    {
        lastButtonEvent = millis();
        if (upPressed)
            selectedItem = (selectedItem + MENU_COUNT - 1) % MENU_COUNT;
        else
            selectedItem = (selectedItem + 1) % MENU_COUNT;
        drawMenu();
    }
}

void updateMotorTest()
{
    if (millis() - testStepStarted < motorSteps[testStep].durationMs)
        return;

    testStep++;
    if (testStep >= TEST_STEP_COUNT)
    {
        stopAndShow("Motor test complete");
        return;
    }
    startMotorTestStep();
}

void updateLineFollower()
{
    readSensors();

    const int weights[5] = {-2, -1, 0, 1, 2};
    int blackCount = 0;
    int weightedSum = 0;

    for (int i = 0; i < 5; i++)
    {
        if (sensorValues[i] == BLACK_LEVEL)
        {
            blackCount++;
            weightedSum += weights[i];
        }
    }

    // Stop when the line is lost or an uncertain intersection appears.
    if (blackCount == 0 || blackCount >= 4)
    {
        stopMotors();
    }
    else
    {
        // Keep fractional positions by multiplying before division.
        int correction = (weightedSum * TURN_GAIN) / blackCount;
        if (REVERSE_SENSOR_ORDER)
            correction = -correction;

        int leftPWM = constrain(BASE_SPEED + correction, 0, MAX_SPEED);
        int rightPWM = constrain(BASE_SPEED - correction, 0, MAX_SPEED);
        driveMotors(leftPWM, rightPWM);
    }

    if (millis() - lastSerialPrint >= 250)
    {
        printSensors();
        if (blackCount == 0)
            Serial.println("Line lost: stopped");
        else if (blackCount >= 4)
            Serial.println("Wide line/intersection: stopped");
        lastSerialPrint = millis();
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);

    setupSensors();
    setupMotors(); // Motors disabled until you choose an option.

    Wire.begin(SDA_PIN, SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("OLED initialization failed!");
        stopMotors();
        while (true)
            delay(1000);
    }

    Serial.println("MazeBot ready: motors OFF");
    drawMenu();
}

void loop()
{
    handleButtons();

    if (mode == SENSORS && millis() - lastScreenUpdate >= 120)
    {
        readSensors();
        drawSensorScreen();
        lastScreenUpdate = millis();
    }
    else if (mode == MOTOR_TEST)
        updateMotorTest();
    else if (mode == FOLLOWING)
        updateLineFollower();

    delay(10);
}