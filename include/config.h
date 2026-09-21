#ifndef CONFIG_H
#define CONFIG_H

// --- HARDWARE PIN MAPPING ---
#define PIN_DHT             4
#define PIN_SOIL_MOISTURE   34
#define PIN_LDR             36
#define PIN_RELAY           23
#define PIN_NEOPIXEL        27
#define PIN_BUZZER          18

// --- I2C DISPLAY CONFIGURATION ---
#define I2C_SDA             21
#define I2C_SCL             22
#define LCD_ADDR            0x27
#define LCD_COLS            16
#define LCD_ROWS            2

// // --- BUSINESS LOGIC THRESHOLDS ---
#define SOIL_DRY_THRESHOLD   400
#define TEMP_HIGH_THRESHOLD 35.0

#endif