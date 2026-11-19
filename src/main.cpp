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

// Sensor check is deliberately separate from line following. The TCRT5000
// outputs are digital, so software cannot change the module's threshold.
// Record a reading only after three consecutive equal samples (60 ms).
const unsigned long SENSOR_SAMPLE_MS = 20;
const uint8_t SENSOR_STABLE_SAMPLES = 3;
unsigned long lastSensorSample = 0;
int sensorLastRaw[5] = {-1, -1, -1, -1, -1};
uint8_t sensorSameCount[5] = {0, 0, 0, 0, 0};
int sensorStable[5] = {-1, -1, -1, -1, -1};
bool sensorSawLow[5] = {false, false, false, false, false};
bool sensorSawHigh[5] = {false, false, false, false, false};
bool sensorSawSolo[5] = {false, false, false, false, false};

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

void resetSensorDiagnostics()
{
    // UP resets the evidence so each channel can be retested independently.
    for (int i = 0; i < 5; i++)
    {
        sensorLastRaw[i] = -1;
        sensorSameCount[i] = 0;
        sensorStable[i] = -1;
        sensorSawLow[i] = false;
        sensorSawHigh[i] = false;
        sensorSawSolo[i] = false;
    }
    readSensors();
    lastSensorSample = millis();
    lastScreenUpdate = 0;
    lastSerialPrint = 0;
}

void sampleSensorDiagnostics()
{
    readSensors();

    for (int i = 0; i < 5; i++)
    {
        if (sensorValues[i] == sensorLastRaw[i])
        {
            if (sensorSameCount[i] < SENSOR_STABLE_SAMPLES)
                sensorSameCount[i]++;
        }
        else
        {
            sensorLastRaw[i] = sensorValues[i];
            sensorSameCount[i] = 1;
        }

        if (sensorSameCount[i] >= SENSOR_STABLE_SAMPLES)
        {
            sensorStable[i] = sensorValues[i];
            if (sensorStable[i] == LOW)
                sensorSawLow[i] = true;
            else
                sensorSawHigh[i] = true;
        }
    }

    // SOLO means this one channel was different while all other four agreed.
    // It is useful for detecting an incorrectly wired/shared input.
    for (int i = 0; i < 5; i++)
    {
        if (sensorStable[i] < 0)
            continue;

        int otherValue = -1;
        bool othersAgree = true;
        for (int j = 0; j < 5; j++)
        {
            if (j == i)
                continue;
            if (sensorStable[j] < 0)
            {
                othersAgree = false;
                break;
            }
            if (otherValue < 0)
                otherValue = sensorStable[j];
            else if (sensorStable[j] != otherValue)
            {
                othersAgree = false;
                break;
            }
        }
        if (othersAgree && sensorStable[i] != otherValue)
            sensorSawSolo[i] = true;
    }
}

void drawSensorScreen()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(12, 0);
    display.println("SENSOR CHECK");

    display.setCursor(0, 12);
    display.print("CH");
    display.setCursor(0, 23);
    display.print("LIVE");
    display.setCursor(0, 34);
    display.print("BOTH");
    display.setCursor(0, 45);
    display.print("SOLO");

    for (int i = 0; i < 5; i++)
    {
        int x = 38 + i * 17;
        display.setCursor(x, 12);
        display.print(i + 1);
        display.setCursor(x, 23);
        display.print(sensorStable[i] < 0 ? '-' : char('0' + sensorStable[i]));
        display.setCursor(x, 34);
        display.print(sensorSawLow[i] && sensorSawHigh[i] ? 'Y' : '-');
        display.setCursor(x, 45);
        display.print(sensorSawSolo[i] ? 'Y' : '-');
    }

    display.setCursor(0, 56);
    display.print("UP=Reset SEL=Back");
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
        resetSensorDiagnostics();
        drawSensorScreen();
        Serial.println("Sensor test: move a narrow dark card under each sensor.");
        Serial.println("BOTH = stable LOW and HIGH seen; SOLO = independently different.");
        Serial.println("UP resets the check. SELECT returns to menu.");
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

    if (mode == SENSORS && upPressed)
    {
        lastButtonEvent = millis();
        resetSensorDiagnostics();
        drawSensorScreen();
        Serial.println("Sensor evidence reset.");
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

void updateSensorDiagnostics()
{
    unsigned long now = millis();

    if (now - lastSensorSample >= SENSOR_SAMPLE_MS)
    {
        lastSensorSample = now;
        sampleSensorDiagnostics();
    }

    if (now - lastScreenUpdate >= 120)
    {
        drawSensorScreen();
        lastScreenUpdate = now;
    }

    if (now - lastSerialPrint >= 500)
    {
        printSensors(); // Current raw levels for troubleshooting.
        Serial.print("BOTH: ");
        for (int i = 0; i < 5; i++)
            Serial.print(sensorSawLow[i] && sensorSawHigh[i] ? 'Y' : '-');
        Serial.print(" | SOLO: ");
        for (int i = 0; i < 5; i++)
            Serial.print(sensorSawSolo[i] ? 'Y' : '-');
        Serial.println();
        lastSerialPrint = now;
    }
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

    if (mode == SENSORS)
        updateSensorDiagnostics();
    else if (mode == MOTOR_TEST)
        updateMotorTest();
    else if (mode == FOLLOWING)
        updateLineFollower();

    delay(10);
}