#pragma once
// ============================================================
//  boards.h — Pin definitions for the LilyGo T3 / TBeam family
//  Only SX1276-based modules are supported here.
//
//  ► Uncomment EXACTLY ONE board define below ◄
// ============================================================

// #define LILYGO_TBeam_V0_7
// #define LILYGO_TBeam_V1_X
// #define LILYGO_T3_V1_0
// #define LILYGO_T3_V1_3
// #define LILYGO_T3_V1_6
// #define LILYGO_T3_V1_6_TCXO
// #define LILYGO_T3_V2_0
// #define LILYGO_T3_S3_V1_0
#define  LILYGO_T3_S3_V1_2          // ← active board
// #define LILYGO_T3_S3_E_PAPER_V_1_0

// Default frequency region is 868 MHz (EU).
// To change region, edit lmic_project_config.h (CFG_eu868 / CFG_us915 …).

// ── Sentinel for unused pins ──────────────────────────────────
#define UNUSE_PIN 0

// ─────────────────────────────────────────────────────────────
//  TBeam V0.7
// ─────────────────────────────────────────────────────────────
#if defined(LILYGO_TBeam_V0_7)
#define GPS_RX_PIN          12
#define GPS_TX_PIN          15
#define BUTTON_PIN          39
#define BUTTON_PIN_MASK     GPIO_SEL_39
#define I2C_SDA             21
#define I2C_SCL             22

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       23
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      32

#define BOARD_LED           14
#define LED_ON              HIGH
#define LED_OFF             LOW

#define GPS_BAUD_RATE       9600
#define HAS_GPS
#define HAS_DISPLAY

// ─────────────────────────────────────────────────────────────
//  TBeam V1.x  (AXP192 / AXP2101)
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_TBeam_V1_X)
#define GPS_RX_PIN          34
#define GPS_TX_PIN          12
#define BUTTON_PIN          38
#define BUTTON_PIN_MASK     GPIO_SEL_38
#define I2C_SDA             21
#define I2C_SCL             22
#define PMU_IRQ             35

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       23
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      32

#define BOARD_LED            4
#define LED_ON              LOW
#define LED_OFF             HIGH

#define GPS_BAUD_RATE       9600
#define HAS_GPS
#define HAS_DISPLAY
#define HAS_PMU

// ─────────────────────────────────────────────────────────────
//  T3 V1.0
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_V1_0)
#define I2C_SDA              4
#define I2C_SCL             15
#define OLED_RST            16

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       14
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      32

#define HAS_DISPLAY

// ─────────────────────────────────────────────────────────────
//  T3 V1.3
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_V1_3)
#define I2C_SDA             21
#define I2C_SCL             22
#define OLED_RST            UNUSE_PIN

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       14
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      32

#define ADC_PIN             35
#define HAS_DISPLAY

// ─────────────────────────────────────────────────────────────
//  T3 V1.6
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_V1_6)
#define I2C_SDA             21
#define I2C_SCL             22
#define OLED_RST            UNUSE_PIN

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       23
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      32

#define SDCARD_MOSI         15
#define SDCARD_MISO          2
#define SDCARD_SCLK         14
#define SDCARD_CS           13

#define BOARD_LED           25
#define LED_ON              HIGH
#define LED_OFF             LOW

#define ADC_PIN             35
#define HAS_SDCARD
#define HAS_DISPLAY

// ─────────────────────────────────────────────────────────────
//  T3 V1.6.1 TCXO  (DIO1 repurposed as TCXO enable)
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_V1_6_TCXO)
#define I2C_SDA             21
#define I2C_SCL             22
#define OLED_RST            UNUSE_PIN

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       23
// DIO1 is wired to the radio's TCXO enable on this variant.
#define RADIO_TCXO_ENABLE   33
#define RADIO_BUSY_PIN      32

#define SDCARD_MOSI         15
#define SDCARD_MISO          2
#define SDCARD_SCLK         14
#define SDCARD_CS           13

#define BOARD_LED           25
#define LED_ON              HIGH
#define LED_OFF             LOW

#define ADC_PIN             35
#define HAS_SDCARD
#define HAS_DISPLAY

// ─────────────────────────────────────────────────────────────
//  T3 V2.0
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_V2_0)
#define I2C_SDA             21
#define I2C_SCL             22
#define OLED_RST            UNUSE_PIN

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       14
#define RADIO_DIO1_PIN      UNUSE_PIN
#define RADIO_BUSY_PIN      UNUSE_PIN

#define SDCARD_MOSI         15
#define SDCARD_MISO          2
#define SDCARD_SCLK         14
#define SDCARD_CS           13

#define BOARD_LED            0
#define LED_ON              LOW
#define LED_OFF             HIGH

#define HAS_DISPLAY
#define HAS_SDCARD

// ─────────────────────────────────────────────────────────────
//  T3-S3 V1.0 / V1.2  (ESP32-S3 + SX1276)
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_S3_V1_0) || defined(LILYGO_T3_S3_V1_2)
#define I2C_SDA             18
#define I2C_SCL             17
#define OLED_RST            UNUSE_PIN

// Radio — FSPI bus on ESP32-S3
#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN       3
#define RADIO_MOSI_PIN       6
#define RADIO_CS_PIN         7
#define RADIO_RST_PIN        8
#define RADIO_BUSY_PIN      34
#define RADIO_DIO1_PIN      33

// SX1276-specific interrupt pins
#define RADIO_DIO0_PIN       9   // TX done / RX done
#define RADIO_DIO3_PIN      21
#define RADIO_DIO4_PIN      10
#define RADIO_DIO5_PIN      36

// SD card — HSPI bus
#define SDCARD_MOSI         11
#define SDCARD_MISO          2
#define SDCARD_SCLK         14
#define SDCARD_CS           13

#define BOARD_LED           37
#define LED_ON              HIGH
#define LED_OFF             LOW

#define BAT_ADC_PIN          1
#define BUTTON_PIN           0

#define HAS_SDCARD
#define HAS_DISPLAY

// ─────────────────────────────────────────────────────────────
//  T3-S3 E-Paper V1.0
// ─────────────────────────────────────────────────────────────
#elif defined(LILYGO_T3_S3_E_PAPER_V_1_0)
#define I2C_SDA             18
#define I2C_SCL             17
#define OLED_RST            UNUSE_PIN

#define EDP_BUSY_PIN        48
#define EDP_RSET_PIN        47
#define EDP_DC_PIN          16
#define EDP_CS_PIN          15
#define EDP_CLK_PIN         14
#define EDP_MOSI_PIN        11
#define EDP_MISO_PIN        -1

#define RADIO_SCLK_PIN       5
#define RADIO_MISO_PIN       3
#define RADIO_MOSI_PIN       6
#define RADIO_CS_PIN         7
#define RADIO_RST_PIN        8
#define RADIO_BUSY_PIN      34
#define RADIO_DIO1_PIN      33

#define RADIO_DIO0_PIN       9
#define RADIO_DIO3_PIN      21
#define RADIO_DIO4_PIN      10
#define RADIO_DIO5_PIN      36

#define SDCARD_MOSI         EDP_MOSI_PIN
#define SDCARD_SCLK         EDP_CLK_PIN
#define SDCARD_MISO          2
#define SDCARD_CS           13

#define BOARD_LED           37
#define LED_ON              HIGH
#define LED_OFF             LOW

#define BAT_ADC_PIN          1
#define BUTTON_PIN           0

#define HAS_SDCARD
#define EDP_DISPLAY

#else
#error "No board selected — uncomment one #define at the top of boards.h"
#endif
