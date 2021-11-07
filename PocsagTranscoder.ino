/*
 * POCSAG transmitter example for Arduino Nano
 * 
 * RF7021SE - Arduino wiring:
 * VCC      - VCC (Don't forget to convert your Arduino to run on 3.3V first!)
 * CE       - D6
 * SLE      - D7
 * SDATA    - D8
 * SREAD    - D9
 * SCLK     - D10
 * TxRxCLK  - D11
 * TxRxData - D12
 * GND      - GND
 * 
 * Serial console commands:
 * d or D XXX - set frequency deviation in Hz (e.g. "d 4500" sets deviation to 4.5KHz)
 * f or F XXX - set frequency in Hz (e.g. "f 433920000" sets frequency to 433.920MHz)
 * i or I X - enable/disable data inversion (e.g. "i 0" turns inversion off, "i 1" - turns on)
 * r or R XXXX - set data rate (512, 1200 or 2400, e.g. "r 512" sets rate to 512kbps)
 * x or X - run transmitter test (by default transmit 0101... pattern on selected frequency, see RF_TEST_MODE and RF_TEST_DURATION)
 * c or C XXXX - set CAP code (e.g. "c 1234567")
 * s or S X - set message source code (0 to 3, e.g. "s 3")
 * e or E X - set encoding (0 to 3, e.g. "e 0" sets Latin Motorola encoding)
 * n or N XXXX - send numeric message (e.g. "n U *123*456*789")
 * m or M XXXX - send alphanumeric message (e.g. "m Hello World")
 * t or T - send tone message
 *
 * Copyright (c) 2021, SinuX. All rights reserved.
 * 
 * This library is distributed "as is" without any warranty.
 * See MIT License for more details.
 */
#include "src/Rf7021.h"
#include "src/PocsagEncoder.h"

// RF7021 Wiring
#define RF_CE_PIN      6
#define RF_SLE_PIN     7
#define RF_SDATA_PIN   8
#define RF_SREAD_PIN   9
#define RF_SCLK_PIN    10
#define RF_TX_CLK_PIN  11
#define RF_TX_DATA_PIN 12

// RF7021 settings
#define RF_TCXO 12288000UL      // 12.288 MHz TCXO
#define RF_PRINT_ADF_INFO       // Read and print chip revision, temperature and voltage at startup
#define RF_TEST_MODE 4          // Test mode - "010101..." pattern
#define RF_TEST_DURATION 10000  // Test duration 10 seconds

// Pocsag default settings
#define PCSG_DEFAULT_DATA_RATE 1200        // 1200bps
#define PCSG_DEFAULT_FREQ      433000000UL // 433.000 MHz
#define PCSG_DEFAULT_FREQ_DEV  4500        // 4.5 KHz
#define PCSG_DEFAULT_CAP_CODE  1234567     // 7th frame
#define PCSG_DEFAULT_SOURCE    3           // Alphanumeric
#define PCSG_DEFAULT_ENCODING  0           // Latin (Motorola)

Rf7021 rf = Rf7021();
PocsagEncoder pocsagEncoder = PocsagEncoder();

void setup() {
  // Set transmitter control pins
  rf.setCEPin(RF_CE_PIN);
  rf.setSLEPin(RF_SLE_PIN);
  rf.setSDATAPin(RF_SDATA_PIN);
  rf.setSREADPin(RF_SREAD_PIN);
  rf.setSCLKPin(RF_SCLK_PIN);
  rf.setTxRxCLKPin(RF_TX_CLK_PIN);
  rf.setTxRxDataPin(RF_TX_DATA_PIN);

  Serial.begin(9600);

  #ifdef RF_PRINT_ADF_INFO
  // Print ADF readback info
  Serial.print(F("Silicon rev: "));
  Serial.println(rf.getSiliconRev(), HEX);
  Serial.print(F("Temp °C: "));
  Serial.println(rf.getTemp());
  Serial.print(F("Voltage: "));
  Serial.println(rf.getVoltage());
  #endif

  // Apply default frequency and deviation
  rf.setXtalFrequency(RF_TCXO);
  rf.setHasExternalInductor(1);
  rf.setDataRate(PCSG_DEFAULT_DATA_RATE);
  rf.setFrequency(PCSG_DEFAULT_FREQ);
  rf.setFrequencyDeviation(PCSG_DEFAULT_FREQ_DEV);

  // Apply default encoder params
  pocsagEncoder.setCapCode(PCSG_DEFAULT_CAP_CODE);
  pocsagEncoder.setSource(PCSG_DEFAULT_SOURCE);
  pocsagEncoder.setEncodingId(PCSG_DEFAULT_ENCODING);
  //pocsagEncoder.setPrintMessage(1);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'd' || command == 'D') {
      // d or D XXXX - set frequency deviation in Hz
      uint32_t freqDev = Serial.parseInt();
      rf.setFrequencyDeviation(freqDev);
      Serial.print(F("Frequency Deviation: "));
      Serial.print(freqDev / 1000.0, 2);
      Serial.println(F(" KHz"));
    } else if (command == 'f' || command == 'F') {
      // f or F XXXX - set frequency in Hz
      uint32_t freq = Serial.parseInt();
      if (freq >= 80000000 && freq <= 950000000) {
        rf.setFrequency(freq);
        Serial.print(F("Frequency: "));
        Serial.print(freq / 1000000.0, 3);
        Serial.println(F(" MHz"));
      } else {
        Serial.println(F("Wrong frequency, should be 80000000 to 950000000"));
      }
    } else if (command == 'i' || command == 'I') {
      // i or I X - set data inversion (0 - disabled, 1 - enabled)
      byte inv = Serial.parseInt();
      rf.setDataInvertEnabled(inv ? 1 : 0);
      Serial.print(F("Inversion: "));
      Serial.println(inv ? F("enabled") : F("disabled"));
    } else if (command == 'r' || command == 'R') {
      // r or R XXXX - set data rate (512, 1200 or 2400)
      word rate = Serial.parseInt();
      if (rate == 512 || rate == 1200 || rate == 2400) {
        rf.setDataRate(rate);
        Serial.print(F("Data rate: "));
        Serial.println(rate); 
      } else {
        Serial.println(F("Wrong data rate, should be 512, 1200 or 2400"));
      }
    } else if (command == 'x' || command == 'X') {
      // x or X - run transmitter test
      Serial.println(F("Testing..."));
      rf.txTest(RF_TEST_MODE, RF_TEST_DURATION);
      Serial.println(F("Test completed"));
    } else if (command == 'c' || command == 'C') {
      // c or C XXXX - set CAP code
      uint32_t cap = Serial.parseInt();
      pocsagEncoder.setCapCode(cap);
      Serial.print(F("CAP code: "));
      Serial.println(cap);
    } else if (command == 's' || command == 'S') {
      // s or S X - set message source code (0 to 3)
      byte src = Serial.parseInt();
      if (src <= 3) {
        pocsagEncoder.setSource(src);
        Serial.print(F("Source code: "));
        Serial.println(src); 
      } else {
        Serial.println(F("Wrong source code, should be 0, 1, 2 or 3"));
      }
    } else if (command == 'e' || command == 'E') {
      // e or E X - set encoding (0 to 3)
      byte enc = Serial.parseInt();
      if (enc <= 3) {
        pocsagEncoder.setEncodingId(enc);
        Serial.print(F("Encoding: "));
        Serial.print(enc);
        Serial.println(enc == 0 ? F(" (EN Motorola)") : enc == 1 ? F(" (EN/RU Motorola)") : enc == 2 ? F(" (EN/RU Philips)") : enc == 3 ? F(" (RU Motorola)") : F(" (Unknown)"));
      } else {
         Serial.println(F("Wrong encoding, should be 0, 1, 2 or 3"));
      }
    } else if (command == 'n' || command == 'N') {
      // n or N XXXX - send numeric message
      Serial.println(F("Message encoding..."));
      PocsagEncoder::PocsagMessage message = pocsagEncoder.encodeNumeric(Serial.readString());
      Serial.println(F("Sending..."));
      rf.sendMessage(message.dataBytes, message.messageLength);
      Serial.println(F("Message sent"));
    } else if (command == 'm' || command == 'M') {
      // m or M XXXX - send alphanumeric message
      Serial.println(F("Message encoding..."));
      PocsagEncoder::PocsagMessage message = pocsagEncoder.encodeAlphanumeric(Serial.readString());
      Serial.println(F("Sending..."));
      rf.sendMessage(message.dataBytes, message.messageLength);
      Serial.println(F("Message sent"));
    }  else if (command == 't' || command == 'T') {
      // t or T - send tone message
      Serial.println(F("Message encoding..."));
      PocsagEncoder::PocsagMessage message = pocsagEncoder.encodeTone();
      Serial.println(F("Sending..."));
      rf.sendMessage(message.dataBytes, message.messageLength);
      Serial.println(F("Message sent"));
    }
  }
}
