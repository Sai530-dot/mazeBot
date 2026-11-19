# MazeBot virtual ESP32 test bench (Wokwi)

This is a **firmware/electronics** simulator for the existing PlatformIO project. It runs the compiled firmware from `src/main.cpp` on a simulated ESP32-S3. It does **not** physically move a robot or generate IR readings from an image of a track.

## Setup

1. Open the MazeBot repository in **VS Code** (the same folder that contains `platformio.ini`).
2. Install the **PlatformIO IDE** extension and the **Wokwi Simulator** extension.
3. Request/activate a Wokwi VS Code license using the extension's instructions (Wokwi has a trial and published open-source licensing conditions).
4. Switch to the GitHub branch `feat/sensor-diagnostics-2026-11-18`.
5. Run `pio run` from the project root; this produces `.pio/build/freenove_esp32_s3_wroom/firmware.bin`.
6. Press **F1** and select **Wokwi: Start Simulator**.

`wokwi.toml` points at the actual Freenove PlatformIO build. `diagram.json` declares an ESP32-S3 DevKitC virtual stand-in that uses the same chip and GPIO numbers as the physical Freenove ESP32-S3. The two boards have different physical layouts and are not electrically identical.

## Virtual components and pins

| Test bench part | ESP32 pin / mapping | What to do |
| --- | --- | --- |
| SSD1306 OLED | SDA=8, SCL=9 | Menu should appear on the display |
| UP / DOWN / SELECT buttons | 4 / 5 / 6 | Click buttons, or use U / D / S shortcuts |
| IR sensor switches 1..5 | 10, 11, 12, 13, 14 | Flip switches to emulate digital sensor levels |
| Left motor PWM LED | 39 (PWMA) | Brightness approximates PWM output, **not real motor speed** |
| Right motor PWM LED | 40 (PWMB) | Same for right output |
| Logic analyzer channels D0..D6 | 39, 15, 16, 40, 17, 18, 38 | PWMA, AIN1, AIN2, PWMB, BIN1, BIN2, STBY |

The five **slide switches** are **not optical sensors**. Each drives one GPIO directly:
- Right position (initial setting): HIGH (`1`) — emulates white/background, assuming active-low black detection.
- Left position: LOW (`0`) — emulates black/line.
- The polarity assumption is from the current firmware; real TCRT5000 modules may be calibrated differently.

## Test the current firmware

1. Start the simulator and check that **DSA MazeBot** displays on the virtual OLED.
2. Use buttons U/D to navigate, S to select.
3. Choose **Sensor test**, then move switches individually. Observe LIVE, BOTH and SOLO flags on the OLED, and the Serial Monitor output. For a convincing independent-channel test, flip one switch at a time and leave the other four at `1` for at least 100 milliseconds.
4. Press UP inside Sensor test to reset evidence, or SELECT to return to menu.
5. Choose **Motor test**. The left and right LEDs should light in their respective steps. You can inspect PWM and direction pins with the logic analyzer after stopping the simulation.
6. Choose **DFS (line)**, **BFS (line)** or **A* (line)**. These currently all execute the **same basic line follower**; no maze-solving algorithm is yet implemented. Change the sensor switches to simulate a centered line (`11011`), a line toward the left, no line (`11111`), and a wide line (`00000`). The motor outputs should respond or stop as coded.

## Change the code

Edit the normal C++ files under `src/`, run `pio run`, then restart/reload the Wokwi simulation as needed. The virtual test bench uses the same compiled firmware, not a separate rewrite.

## Limits and next steps

- Wokwi can validate GPIO, I2C/OLED, menu states and PWM control signals. It cannot by itself verify real TCRT5000 detection distance, battery sag, physical motor torque, traction or whether the wheels actually trace a course.
- The LEDs are direct electrical indicators of PWM pins. The **TB6612FNG driver and DC motors are not modeled** in this bench.
- For actual robot movement through a drawn maze, add a 2D kinematic simulator or a Webots robot model. Ideally the maze-navigation decisions should live in a hardware-independent C++ module that can be exercised by desktop tests and then called from the ESP32 firmware.
- `wokwi-cli lint` can be used to validate `diagram.json` if the CLI is installed. This draft diagram has not yet been run in Wokwi and must be validated in the VS Code simulator.
