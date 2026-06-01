#pragma once
// ============================================================
//  utilities.h  —  Board init for LilyGo T3 family
//  Optimised for T3-S3 V1.2 (ESP32-S3 + SX1276)
// ============================================================

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "boards.h"

// ── SD card ──────────────────────────────────────────────────
#ifdef HAS_SDCARD
#include <FS.h>
#include <SD.h>
// Use HSPI bus for SD so it doesn't clash with the radio's FSPI/SPI bus.
static SPIClass SDSPI(HSPI);
#endif  // HAS_SDCARD

// ── OLED display (SSD1306 128×64 via I²C) ───────────────────
#ifdef HAS_DISPLAY
#include <U8g2lib.h>

#ifndef DISPLAY_MODEL
#define DISPLAY_MODEL U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#endif

DISPLAY_MODEL *u8g2 = nullptr;

#elif defined(EDP_DISPLAY)
// ── E-paper display (T3-S3 E-Paper variant only) ────────────
#include <GxEPD.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "Adafruit_GFX.h"
#include GxEPD_BitmapExamples

static SPIClass SDSPI(HSPI);   // shared with SD on this variant
GxIO_Class io(SDSPI, EDP_CS_PIN, EDP_DC_PIN, EDP_RSET_PIN);
GxEPD_Class display(io, EDP_RSET_PIN, EDP_BUSY_PIN);
#endif  // display type

// ── PMU (AXP192 / AXP2101 — TBeam variants only) ────────────
#if defined(HAS_PMU)
#include "XPowersLib.h"

#ifndef PMU_WIRE_PORT
#define PMU_WIRE_PORT Wire
#endif

static XPowersLibInterface *PMU = nullptr;
static volatile bool pmuInterruptFlag = false;

static void IRAM_ATTR onPmuIrq() { pmuInterruptFlag = true; }

static bool initPMU() {
    // Try AXP2101 first, then AXP192
    const struct { XPowersLibInterface *obj; const char *name; } candidates[] = {
        { new XPowersAXP2101(PMU_WIRE_PORT), "AXP2101" },
        { new XPowersAXP192(PMU_WIRE_PORT),  "AXP192"  },
    };

    for (auto &c : candidates) {
        if (c.obj->init()) {
            PMU = c.obj;
            Serial.printf("[PMU] Found %s\n", c.name);
        } else {
            delete c.obj;
        }
        if (PMU) break;
    }

    if (!PMU) {
        Serial.println("[PMU] No power management IC found");
        return false;
    }

    PMU->setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

    pinMode(PMU_IRQ, INPUT_PULLUP);
    attachInterrupt(PMU_IRQ, onPmuIrq, FALLING);

    if (PMU->getChipModel() == XPOWERS_AXP192) {
        // Power rails for TBeam V1.x
        PMU->setPowerChannelVoltage(XPOWERS_LDO2,  3300); // LoRa
        PMU->setPowerChannelVoltage(XPOWERS_LDO3,  3300); // GPS
        PMU->setPowerChannelVoltage(XPOWERS_DCDC1, 3300); // OLED
        PMU->enablePowerOutput(XPOWERS_LDO2);
        PMU->enablePowerOutput(XPOWERS_LDO3);
        PMU->enablePowerOutput(XPOWERS_DCDC1);
        PMU->setProtectedChannel(XPOWERS_DCDC1);   // protect OLED rail
        PMU->setProtectedChannel(XPOWERS_DCDC3);   // protect ESP32 rail
        PMU->disablePowerOutput(XPOWERS_DCDC2);

        PMU->disableIRQ(XPOWERS_AXP192_ALL_IRQ);
        PMU->enableIRQ(XPOWERS_AXP192_VBUS_REMOVE_IRQ  |
                       XPOWERS_AXP192_VBUS_INSERT_IRQ  |
                       XPOWERS_AXP192_BAT_CHG_DONE_IRQ |
                       XPOWERS_AXP192_BAT_CHG_START_IRQ|
                       XPOWERS_AXP192_BAT_REMOVE_IRQ   |
                       XPOWERS_AXP192_BAT_INSERT_IRQ   |
                       XPOWERS_AXP192_PKEY_SHORT_IRQ);

    } else if (PMU->getChipModel() == XPOWERS_AXP2101) {
#if defined(CONFIG_IDF_TARGET_ESP32)
        // Disable unused rails
        const uint8_t offChannels[] = {
            XPOWERS_DCDC2, XPOWERS_DCDC3, XPOWERS_DCDC4, XPOWERS_DCDC5,
            XPOWERS_ALDO1, XPOWERS_ALDO4,
            XPOWERS_BLDO1, XPOWERS_BLDO2,
            XPOWERS_DLDO1, XPOWERS_DLDO2,
        };
        for (uint8_t ch : offChannels) PMU->disablePowerOutput(ch);

        PMU->setPowerChannelVoltage(XPOWERS_VBACKUP, 3300); // GPS RTC
        PMU->enablePowerOutput(XPOWERS_VBACKUP);
        PMU->setProtectedChannel(XPOWERS_DCDC1);            // ESP32

        PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);   // LoRa
        PMU->enablePowerOutput(XPOWERS_ALDO2);
        PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);   // GPS
        PMU->enablePowerOutput(XPOWERS_ALDO3);
#endif

#if defined(LILYGO_TBeamS3_SUPREME_V3_0)
        PMU->setPowerChannelVoltage(XPOWERS_ALDO4, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO4);
        PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO3);

        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
            Serial.println("[PMU] Cold boot: cycling ALDO/BLDO");
            PMU->disablePowerOutput(XPOWERS_ALDO1);
            PMU->disablePowerOutput(XPOWERS_ALDO2);
            PMU->disablePowerOutput(XPOWERS_BLDO1);
            delay(250);
        }

        PMU->setPowerChannelVoltage(XPOWERS_ALDO1, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO1);
        PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO2);
        PMU->setPowerChannelVoltage(XPOWERS_BLDO1, 3300);
        PMU->enablePowerOutput(XPOWERS_BLDO1);
        PMU->setPowerChannelVoltage(XPOWERS_BLDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_BLDO2);
        PMU->setPowerChannelVoltage(XPOWERS_DCDC3, 3300);
        PMU->enablePowerOutput(XPOWERS_DCDC3);
        PMU->setPowerChannelVoltage(XPOWERS_DCDC4, XPOWERS_AXP2101_DCDC4_VOL2_MAX);
        PMU->enablePowerOutput(XPOWERS_DCDC4);
        PMU->setPowerChannelVoltage(XPOWERS_DCDC5, 3300);
        PMU->enablePowerOutput(XPOWERS_DCDC5);

        PMU->disablePowerOutput(XPOWERS_DCDC2);
        PMU->disablePowerOutput(XPOWERS_DLDO1);
        PMU->disablePowerOutput(XPOWERS_DLDO2);
        PMU->disablePowerOutput(XPOWERS_VBACKUP);

        PMU->setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_500MA);
        PMU->setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);

        PMU->disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
        PMU->clearIrqStatus();
        PMU->enableIRQ(XPOWERS_AXP2101_BAT_INSERT_IRQ  |
                       XPOWERS_AXP2101_BAT_REMOVE_IRQ  |
                       XPOWERS_AXP2101_VBUS_INSERT_IRQ |
                       XPOWERS_AXP2101_VBUS_REMOVE_IRQ |
                       XPOWERS_AXP2101_PKEY_SHORT_IRQ  |
                       XPOWERS_AXP2101_PKEY_LONG_IRQ   |
                       XPOWERS_AXP2101_BAT_CHG_DONE_IRQ|
                       XPOWERS_AXP2101_BAT_CHG_START_IRQ);
#endif  // LILYGO_TBeamS3_SUPREME_V3_0
    }

    PMU->enableSystemVoltageMeasure();
    PMU->enableVbusVoltageMeasure();
    PMU->enableBattVoltageMeasure();
    // TS-pin measurement must be disabled on boards without a thermistor
    // otherwise charging behaves abnormally.
    PMU->disableTSPinMeasure();

    PMU->setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);
    Serial.println("[PMU] Init OK");
    return true;
}
#else
// Stubs so the rest of the code compiles unchanged on non-PMU boards.
inline void initPMU()          {}
inline void disablePeripherals() {}
#endif  // HAS_PMU

// ── Main board initialisation ─────────────────────────────────
void initBoard() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) ;   // wait for USB CDC (ESP32-S3)
    Serial.println("[Board] initBoard()");

    // Radio SPI bus (FSPI on S3 — pins from boards.h)
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);

    // I²C for OLED (and optionally PMU)
#if defined(HAS_DISPLAY) || defined(HAS_PMU)
    Wire.begin(I2C_SDA, I2C_SCL);
#endif

#ifdef I2C1_SDA
    Wire1.begin(I2C1_SDA, I2C1_SCL);
#endif

    // TCXO enable pin (T3 V1.6.1 TCXO variant only)
#ifdef RADIO_TCXO_ENABLE
    pinMode(RADIO_TCXO_ENABLE, OUTPUT);
    digitalWrite(RADIO_TCXO_ENABLE, HIGH);
#endif

    // GPS UART
#ifdef HAS_GPS
    Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
#endif

    // OLED hardware reset (only needed on boards that wire the RST pin)
#if defined(OLED_RST) && (OLED_RST != 0)
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, HIGH); delay(5);
    digitalWrite(OLED_RST, LOW);  delay(20);
    digitalWrite(OLED_RST, HIGH); delay(10);
#endif

    initPMU();

    // Board LED — T-Beam V1.0/1.1 pulls LOW to turn on; release hold first.
#ifdef BOARD_LED
#if LED_ON == LOW
    gpio_hold_dis(GPIO_NUM_4);
#endif
    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LED_OFF);   // start OFF; .ino controls it
#endif

    // ── OLED init ────────────────────────────────────────────
#ifdef HAS_DISPLAY
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0) {
        Serial.println("[Board] OLED detected at 0x3C");
        u8g2 = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
        u8g2->begin();
        u8g2->setFlipMode(0);
        u8g2->setFontMode(1);       // transparent background
        u8g2->setDrawColor(1);
        u8g2->setFontDirection(0);
        u8g2->setFont(u8g2_font_inb19_mr);
        u8g2->firstPage();
        do {
            u8g2->drawStr(0, 30, "T3-S3");
            u8g2->drawHLine(2, 35, 60);
            u8g2->setFont(u8g2_font_inb19_mf);
            u8g2->drawStr(0, 60, "LoRaWAN");
        } while (u8g2->nextPage());
        u8g2->setFont(u8g2_font_fur11_tf);
        delay(2000);
        u8g2->clearBuffer();
        u8g2->sendBuffer();
    } else {
        Serial.println("[Board] OLED not found — continuing without display");
    }
#endif  // HAS_DISPLAY

    // ── SD card init ─────────────────────────────────────────
#ifdef HAS_SDCARD
#ifdef HAS_DISPLAY
    if (u8g2) u8g2->setFont(u8g2_font_ncenB08_tr);
#endif

    pinMode(SDCARD_MISO, INPUT_PULLUP);
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);

#ifdef EDP_DISPLAY
    display.init();
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0,
                              GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    Serial.println("[Board] E-paper initialised");
#endif

    if (!SD.begin(SDCARD_CS, SDSPI)) {
        Serial.println("[Board] SD card FAILED");
#ifdef HAS_DISPLAY
        if (u8g2) {
            u8g2->clearBuffer();
            u8g2->setCursor(0, 16);
            u8g2->print("SDCard FAILED");
            u8g2->sendBuffer();
        }
#endif
    } else {
        float cardGB = SD.cardSize() / (1024.0f * 1024.0f * 1024.0f);
        Serial.printf("[Board] SD card OK — %.2f GB\n", cardGB);
#ifdef HAS_DISPLAY
        if (u8g2) {
            u8g2->clearBuffer();
            u8g2->setCursor(0, 16);
            u8g2->printf("SD: %.2f GB", cardGB);
            u8g2->sendBuffer();
        }
#endif
    }
    delay(2000);
    // Clear SD init message so LoRaWAN status can use the screen
#ifdef HAS_DISPLAY
    if (u8g2) { u8g2->clearBuffer(); u8g2->sendBuffer(); }
#endif
#endif  // HAS_SDCARD

    Serial.println("[Board] initBoard() done");
}
