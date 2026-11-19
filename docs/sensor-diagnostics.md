# Sensor diagnostic procedure

This stage validates the existing **five-channel digital TCRT5000** array before maze planning. It does not require tape or a physical maze.

## Before powering the motors
- Raise the wheels off the table, or unplug motor power. Sensor Check does not command the motors.
- Check that each sensor faces the surface at a consistent, close distance; adjust the bracket or mounting height. A high-mounted array may detect nothing.
- Use a matte **white card** and a matte **black card**. Avoid glossy surfaces. Keep a stable distance while switching colors.
- If changing between white and black produces the same reading, adjust the sensor module's potentiometer (if fitted). ESP32 digital reads cannot calibrate a comparator's threshold.

## Procedure
1. Build and upload using `pio run -t upload` in the project root.
2. Open `pio device monitor -b 115200`.
3. From the OLED menu choose **Sensor test**.
4. Look at the OLED: each channel has **LIVE**, **BOTH**, and **SOLO** indicators.
   - **LIVE** = last stable digital reading, `0` or `1`. A dash means not yet stable.
   - **BOTH = Y** after this channel has produced *both* stable LOW and stable HIGH since reset.
   - **SOLO = Y** after this channel was the only one with a different stable reading while the other four matched. This is evidence of independent response, not proof of every kind of wiring fault.
5. Press **UP** to reset accumulated evidence.
6. With all five facing white, move a narrow black card under **only channel 1** long enough for the readings to stabilize (at least ~100 ms). Move it away; do the same for channels 2–5, individually. Keep the other four sensors over white.
7. Ideally all five channels eventually show `BOTH Y` and `SOLO Y`. Record any that don't and report the serial output. **SELECT** returns to the menu.

## Troubleshooting
- **All five switch together:** check shared wiring, ground, sensor height, and whether the whole card is affecting all channels.
- **Only HIGH or only LOW:** inspect that channel's VCC/GND/OUT wiring; adjust the module threshold and distance.
- **BOTH Y but SOLO -:** the sensor changes, but the test has not observed it differing alone. Repeat using a narrower black target.
- **Unstable readings:** hold the target steady and reduce variation in distance and ambient lighting.
- **BLACK label incorrect in line following:** the current program assumes black corresponds to LOW. Verify with Serial before changing `BLACK_LEVEL`.

## What this stage does not prove
It does not confirm line-following stability, intersection handling, wheel calibration, DFS/BFS/A* or autonomous maze solving. Those require a repeatable physical test course and subsequent navigation code.
