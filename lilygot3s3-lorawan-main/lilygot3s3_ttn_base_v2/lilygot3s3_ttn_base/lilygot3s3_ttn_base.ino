#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include "utilities.h"

// --- VARIABLES INTERNAS DEL GPS ---
float gpsLat = 0.0;
float gpsLon = 0.0;
bool gpsFix = false;

// --- SECTION 1 - CREDENTIALS (Tus credenciales reales guardadas) ---
// AppEUI (JoinEUI) — leave all-zeros if TTN v3 does not require it
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// DevEUI — For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
// This MUST be in little endian format. (lsb in TTN)
static const u1_t PROGMEM DEVEUI[8] = {0xE5, 0x79, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };

// AppKey — paste from TTN, selecting "MSB" byte order
static const u1_t PROGMEM APPKEY[16] = {0x6C, 0x5D, 0x2C, 0x8A, 0x74, 0xC2, 0x13, 0x57, 0xBD, 0x95, 0x9A, 0x93, 0x8B, 0x3B, 0xCA, 0x25 };

// These three functions are called by the LMIC library — do not rename them
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }


// --- CONFIGURACIÓN DE PINES HARDWARE SERIE GPS ---
#define GPS_RX_PIN 39 // Cable amarillo (TX1 del GPS)
#define GPS_TX_PIN 38 // Cable negro (RX1 del GPS)
#define GPS_BAUDRATE 38400 

HardwareSerial GPSserial(1);
TinyGPSPlus gps;

// --- SECTION 2 - SETTINGS ---
static const unsigned TX_INTERVAL = 60; // Intervalo de 60s como tenías configurado
static osjob_t sendJob;
unsigned long lastDebugTime = 0;

// MAPEO SOLUCIONADO: Vinculamos los nombres reales de tu archivo boards.h
const lmic_pinmap lmic_pins = {
    .nss = RADIO_CS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RADIO_RST_PIN,
    .dio = {RADIO_DIO0_PIN, RADIO_DIO1_PIN, LMIC_UNUSED_PIN},
};

void do_send(osjob_t* j);

// --- MANEJADOR DE EVENTOS LORAWAN NATIVO ---
void onEvent (ev_t ev) {
    switch(ev) {
        case EV_JOINING:
            Serial.println(F("Joining TTN..."));
            break;
        case EV_JOINED:
            Serial.println(F("Joined! ✓"));
            LMIC_setLinkCheckMode(0);
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("Join FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("Packet #1 sent"));
            if (LMIC.txrxFlags & TXRX_ACK) Serial.println(F("Received ack"));
            os_setTimedCallback(&sendJob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        default:
            break;
    }
}

// --- CONSTRUCCIÓN DINÁMICA DEL PAQUETE CON TUS COORDENADAS ---
void do_send(osjob_t* j){
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        int32_t latitude = 0;
        int32_t longitude = 0;
        uint16_t satellites = gps.satellites.value();

        if (gps.location.isValid()) {
            latitude = gps.location.lat() * 1000000;
            longitude = gps.location.lng() * 1000000;
            Serial.printf("Enviando paquete: GPS FIX OK -> Lat: %.6f | Lon: %.6f | Sats: %d\n", 
                          gps.location.lat(), gps.location.lng(), satellites);
        } else {
            Serial.println(F("Enviando paquete: GPS sin fix"));
        }

        byte payload[10];
        payload[0] = (latitude >> 24) & 0xFF;
        payload[1] = (latitude >> 16) & 0xFF;
        payload[2] = (latitude >> 8) & 0xFF;
        payload[3] = latitude & 0xFF;

        payload[4] = (longitude >> 24) & 0xFF;
        payload[5] = (longitude >> 16) & 0xFF;
        payload[6] = (longitude >> 8) & 0xFF;
        payload[7] = longitude & 0xFF;

        payload[8] = (satellites >> 8) & 0xFF;
        payload[9] = satellites & 0xFF;

        LMIC_setTxData2(1, payload, sizeof(payload), 0);
        Serial.print(F("Next TX in ")); Serial.print(TX_INTERVAL); Serial.println(F(" s"));
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 4000) { }

    Serial.println(F("=== T3-S3 V1.2 LoRaWAN OTAA ==="));

    // Inicialización de periféricos nativos de tu placa
    initBoard();
    Serial.println(F("[Board] initBoard() done"));

    // Arrancamos el puerto serie del GPS
    GPSserial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println(F("=== Puerto Serie del GPS Conectado ==="));
    Serial.print(F("TX interval: ")); Serial.print(TX_INTERVAL); Serial.println(F(" s"));

    os_init();
    LMIC_reset();
    LMIC_setAdrMode(false);

    // Arrancamos el ciclo de envíos
    do_send(&sendJob);
}

void loop() {
    os_runloop_once();

    // Alimentamos la librería TinyGPS++ leyendo los caracteres del u-blox
    while (GPSserial.available() > 0) {
        char c = GPSserial.read();
        gps.encode(c);
    }

    // Impresión por monitor serie cada 5 segundos para comprobar satélites
    if (millis() - lastDebugTime > 5000) {
        if (gps.location.isValid()) {
            Serial.printf("[SERIAL] GPS OK -> Lat: %.6f | Lon: %.6f | Satélites: %d\n", 
                          gps.location.lat(), gps.location.lng(), gps.satellites.value());
        } else {
            Serial.printf("[SERIAL] Esperando posición... Satélites en rango: %d | Caracteres leídos: %lu\n", 
                          gps.satellites.value(), gps.charsProcessed());
        }
        lastDebugTime = millis();
    }
}