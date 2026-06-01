// ============================================================
//  lmic_project_config.h
//  MCCI LoRaWAN LMIC — project configuration
//  Target: LilyGo T3-S3 V1.2  (ESP32-S3 + SX1276)
//  Network: The Things Network v3  (EU868)
//
//  FILE LOCATION:
//    .../Arduino/libraries/MCCI_LoRaWAN_LMIC_library/
//                          project_config/lmic_project_config.h
// ============================================================

// Required by the MCCI LMIC HAL for ESP32 (renames hal_init to
// avoid a conflict with the Arduino ESP32 core's own hal_init).
#define hal_init LMICHAL_init

// ── Target MCU ───────────────────────────────────────────────
// Tells LMIC which ESP32 variant it is running on so it picks
// the correct timer resolution and SPI defaults.
#define CFG_esp32_s3 1

// ── Frequency plan ───────────────────────────────────────────
// Uncomment exactly ONE region.  EU868 covers most of Europe
// including Spain.  Change if you are deploying elsewhere.
// #define CFG_eu868 1
#define CFG_us915 1
// #define CFG_us915 1
// #define CFG_au915 1
// #define CFG_as923 1
// #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP  // as923-JP variant
// #define CFG_kr920 1
// #define CFG_in866 1

// ── Radio chip ───────────────────────────────────────────────
// T3-S3 V1.2 carries an SX1276.
#define CFG_sx1276_radio 1

// ── Interrupt vs polling ──────────────────────────────────────
// LMIC_USE_INTERRUPTS is STRONGLY recommended on ESP32-S3.
// Without it LMIC polls DIO pins inside os_runloop_once(), which
// can miss short pulses at high data rates and wastes CPU.
// With it, DIO0/DIO1 fire real GPIO interrupts → lower latency,
// more reliable RX windows, and slightly lower power draw.
//
// Requires DIO0 (GPIO 9) and DIO1 (GPIO 33) to be wired — they
// are on the T3-S3 V1.2, so this is safe to enable.
#define LMIC_USE_INTERRUPTS

// ── LoRaWAN specification version ────────────────────────────
// TTN v3 (The Things Stack) runs LoRaWAN 1.0.3 for OTAA devices.
// Keep this in sync with what your TTN application expects.
#define LMIC_LORAWAN_SPEC_VERSION LMIC_LORAWAN_SPEC_VERSION_1_0_3

// ── Class A only — disable Class B features ──────────────────
// PING and BEACONS are Class B (scheduled downlink) features.
// TTN free tier is Class A only; disabling these saves ~2 KB RAM.
#define DISABLE_PING
#define DISABLE_BEACONS

// ── DeviceTimeReq (LoRaWAN 1.0.3 MAC command) ────────────────
// Allows the device to request the current network time from the
// server. Useful if you need GPS-quality time without a GPS module.
// Set to 1 to enable; TTN v3 supports it.
// Left at 0 here — enable once your application needs it.
#define LMIC_ENABLE_DeviceTimeReq 0

// ── Optional tuning (uncomment as needed) ────────────────────

// Reduce RAM usage by shrinking the TX/RX frame buffer.
// Default is 256 bytes; 64 is enough for most sensor payloads.
// #define MAX_LEN_FRAME 64

// Disable LMIC's internal ADR algorithm completely (alternative
// to calling LMIC_setAdrMode(false) at runtime).
// #define DISABLE_ADR

// Print LMIC debug output to Serial (very verbose — dev only).
// Requires LMIC_DEBUG_LEVEL 1 or 2 in lmic_project_config.h
// and a Serial.begin() before os_init().
// #define LMIC_DEBUG_LEVEL 1
