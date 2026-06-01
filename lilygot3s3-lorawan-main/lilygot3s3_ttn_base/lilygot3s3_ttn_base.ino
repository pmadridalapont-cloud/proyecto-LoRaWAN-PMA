// BEFORE STARTING!
// 1) configure ".../Arduino/libraries/MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h"
// 2) define the board version and model in "boards.h"
//

#include <lmic.h>
#include <hal/hal.h>

#include "utilities.h"
#define BUILTIN_LED BOARD_LED

//
//  LoRaWAN parameters
//
static const u1_t PROGMEM APPEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This EUI must be in little-endian format. 
// For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
// This MUST be in little endian format. (lsb in TTN)
static const u1_t PROGMEM DEVEUI[8]={ 0x01, 0x23, 0x45, 0x67, 0x89, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// PM: This key MUST be in big endian format. (msb in TTN)
static const u1_t PROGMEM APPKEY[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}
//
//  end LoRaWAN parameters
//

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

// PM: Local pin mapping for the Lilygo T3S3
 #define RADIO_CS            RADIO_CS_PIN 		// 7
 #define RADIO_RESET         RADIO_RST_PIN		// 8
 #define RADIO_DIO_0         RADIO_DIO0_PIN		// 9
 #define RADIO_DIO_1         RADIO_DIO1_PIN		// 33
 #define RADIO_DIO_2         LMIC_UNUSED_PIN 	// void

const lmic_pinmap lmic_pins = {
     .nss = RADIO_CS,
     .rxtx = LMIC_UNUSED_PIN,
     .rst = RADIO_RESET,
     .dio = {RADIO_DIO_0, RADIO_DIO_1, RADIO_DIO_2}  
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void oledPrintf(int col, int row, const char* fmt, ...) {
  char msg[50];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);
  Serial.println(msg);

  u8g2->clearBuffer();
  u8g2->drawStr(col, row, msg);
  u8g2->sendBuffer();
}

void oledPrintfbrow(int row, const char* fmt, ...) {
  char msg[50];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);
  Serial.println(msg);

  u8g2->clearBuffer();
  u8g2->drawStr(0, (row+1)*10, msg);
  u8g2->sendBuffer();
}

void onEvent (ev_t ev) {
    long now = os_getTime();
    oledPrintfbrow(0, "Time %lu", now);

    switch(ev) {
        case EV_SCAN_TIMEOUT:
            oledPrintf(0, 7, "EV_SCAN_TIMEOUT");
            break;
        case EV_BEACON_FOUND:
            oledPrintf(0, 7, "EV_BEACON_FOUND");
            break;
        case EV_BEACON_MISSED:
            oledPrintf(0, 7, "EV_BEACON_MISSED");
            break;
        case EV_BEACON_TRACKED:
            oledPrintf(0, 7, "EV_BEACON_TRACKED");
            break;
        case EV_JOINING:
            oledPrintf(0, 7, "EV_JOINING");
            break;
        case EV_JOINED:
            oledPrintf(0, 7, "EV_JOINED");
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0) Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0) Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
              // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            oledPrintf(0, 7, "EV_RFU1");
            break;
        case EV_JOIN_FAILED:
            oledPrintf(0, 7, "EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            oledPrintf(0, 7, "EV_REJOIN_FAILED");
            break;
        case EV_TXCOMPLETE:
            oledPrintf(0, 7, "EV_TXCOMPLETE");
            digitalWrite(BUILTIN_LED, LOW);
            if (LMIC.txrxFlags & TXRX_ACK) {
              oledPrintf(0, 3, "rssi:%d, snr:%1d", LMIC.rssi, LMIC.snr);
              oledPrintf(0, 6, "Received ack");
            }
            if (LMIC.dataLen) {
              oledPrintf(0, 3, "rssi:%d, snr:%1d", LMIC.rssi, LMIC.snr);
              oledPrintf(0, 6, "Received %d", LMIC.dataLen);
              Serial.print("Data:");
              for(size_t i=0; i<LMIC.dataLen; i++) {
                Serial.print(" ");
                printHex2(LMIC.frame[i + LMIC.dataBeg]);
              }
              Serial.println();
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            oledPrintf(0, 7, "EV_LOST_TSYNC");
            break;
        case EV_RESET:
            oledPrintf(0, 7, "EV_RESET");
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            oledPrintf(0, 7, "EV_RXCOMPLETE");
            break;
        case EV_LINK_DEAD:
            oledPrintf(0, 7, "EV_LINK_DEAD");
            break;
        case EV_LINK_ALIVE:
            oledPrintf(0, 7, "EV_LINK_ALIVE");
            break;
        case EV_SCAN_FOUND:
            oledPrintf(0, 7, "EV_SCAN_FOUND");
            break;
        case EV_TXSTART:
            oledPrintf(0, 3, "EV_TXSTART");
            break;
        case EV_TXCANCELED:
            oledPrintf(0, 7, "EV_TXCANCELED");
            break;
        case EV_RXSTART:
            oledPrintf(0, 7, "EV_RXSTART");
            break;
        case EV_JOIN_TXCOMPLETE:
            oledPrintf(0, 7, "EV_JOIN_TXCOMPLETE");
            break;
        default:
            oledPrintf(0, 7, "Unknown event %ud", ev);
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        u1_t confirmed = 1;
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, confirmed);
        Serial.println(F("Packet queued"));
        //digitalWrite(BUILTIN_LED, HIGH);
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup () {
  Serial.begin (115200);
  Serial.println (F ("Starting"));

  initBoard ();
  delay (1500);  // When the power is turned on, a delay is required.

  oledPrintfbrow (2, "Hola! Viva LoRaWAN");

  // LMIC init
  os_init ();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset ();
  LMIC_setAdrMode (false);

  // Start job (sending automatically starts OTAA too)
  do_send (&sendjob);

  pinMode (BUILTIN_LED, OUTPUT);
  digitalWrite (BUILTIN_LED, LOW);
}

void loop () {
  os_runloop_once ();
}

