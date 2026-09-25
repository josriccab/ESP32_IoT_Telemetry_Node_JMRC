/**
 * @file display.h
 * @brief I2C LCD management, non-blocking screen rotation, and telemetry rendering.
 * @author JMRC
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "sensors.h"

// --- I2C LCD INITIALIZATION ---
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// Timing variables for screen rotation
unsigned long lastScreenSwitch = 0;
const unsigned long SCREEN_INTERVAL = 3000; // Switch view every 3 seconds
bool toggleScreen = false;

/**
 * @brief Initializes the I2C bus, LCD display, and shows a startup splash screen.
 */

inline void initDisplay() {
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();

    //Display startup message
    lcd.setCursor(0, 0);
    lcd.print(F("Smart Garden IoT"));
    lcd.setCursor(0, 1);
    lcd.print(F("Initializing..."));
    delay(1500); // Brief pause for readability during boot
    lcd.clear();
}

/**
 * @brief Refreshes sensor telemetry and actuator states on the LCD with a non-blocking layout toggle.
 * @param data Struct containing current environmental sensor readings.
 * @param needsWatering Boolean flag indicating active irrigation status.
 * @param isOverheating Boolean flag indicating a thermal warning state.
 */

inline void updateDisplay(const SensorData& data, bool needsWatering, bool isOverheating) {
    unsigned long currentMillis = millis();

    // Non-blocking screen toggle interval to rotate metrics views
    if (currentMillis - lastScreenSwitch >= SCREEN_INTERVAL) {
        lastScreenSwitch = currentMillis;
        toggleScreen = !toggleScreen;
        lcd.clear(); // Clear residual characters when switching layouts
    }

    // Row 0: Always keeps Temperature and Humidity Telemetry (or overrides with warnings)
    lcd.setCursor(0, 0);
    if (isOverheating) {
        lcd.print(F("! TEMP WARNING !")); 
    } else if (!data.dhtValid) {
        lcd.print(F("DHT22 Error!    ")); // Shows error if DHT fails or disconnects
    } else {
        lcd.print(F("T:"));
        lcd.print(data.temperature, 1);
        lcd.print(F("C H:"));
        lcd.print(data.humidity, 0);
        lcd.print(F("%  ")); // Trailing spaces to clear residual characters
    }

   // Row 1: Alternates between Actuator Status and Light/Soil details
    lcd.setCursor(0, 1);
    if (isOverheating) {
        lcd.print(F("Check Cooling   ")); 
    } else if (toggleScreen) {
        // Screen View A: Soil Moisture & Actuator status (Pump)
        if (!data.soilValid) {
            lcd.print(F("Soil Sens Error ")); // Shows error if soil sensor fails
        } else {
            lcd.print(F("Soil:"));
            lcd.print(data.soilMoisturePercent);
            if (needsWatering) {
                lcd.print(F("% PUMP:ON "));
            } else {
                lcd.print(F("% PUMP:OFF"));
            }
        }
    } else {
        // Screen View B: Light Level Telemetry or LDR Error
        if (!data.ldrValid) {
            lcd.print(F("LDR Error!      ")); // Shows error if LDR disconnects
        } else {
            lcd.print(F("Light Raw: "));
            lcd.print(data.lightLevelRaw);
            lcd.print(F("    "));
        }
    }
}

#endif // DISPLAY_H