#include "rf7021.h"

Rf7021::Rf7021() {
  // Default settings
  rfConfig.xtalFrequency = 19680000UL;  // Default 19.68 MHz TCXO
  rfConfig.hasExternalInductor = 0;     // RF7021SE has no external inductor on L1-L2 pins by default
  rfConfig.xtalBias = 3;                // 0 - 20uA, 1 - 25uA, 2 - 30uA, 3 - 35uA
  rfConfig.cpCurrent = 3;               // 0 - 0.3mA, 1 - 0.9mA, 2 - 1.5mA, 3 - 2.1mA
  rfConfig.modulationScheme = 0;        // 0 - FSK, 1 - GFSK, 2 - 3FSK, 3 - 4FSK, 4 - OFSK, 5 - RCFSK, 6 - RC3FSK, 7 - RC4FSK
  rfConfig.powerAmplifierEnabled = 1;   // 0 - OFF, 1 - ON
  rfConfig.powerAmplifierRamping = 1;   // 0 - OFF, 1 - LOWEST, ..., 7 - HIGHEST
  rfConfig.powerAmplifierBias = 3;      // 0 - 5uA, 1 - 7uA, 2 - 9uA, 3 - 11 uA
  rfConfig.powerAmplifierPower = 63;    // 0 - OFF, 1 - LOWEST, ..., 63 - HIGHEST
  rfConfig.dataInvertType = 2;          // 0 - NONE, 1 - Invert CLK, 2 - Invert DATA, 3 - Invert CLK and DATA
  rfConfig.dataInvertEnabled = 0;       // 0 - OFF, 1 - ON
  rfConfig.rCosineAlpha = 1;            // 0 - 0.5, 1 - 0.7
}

void Rf7021::setTxRxDataPin(byte txRxDataPin) {
  rfConfig.txRxDataPin = txRxDataPin;
  pinMode(txRxDataPin, OUTPUT);
}

void Rf7021::setTxRxCLKPin(byte txRxCLKPin) {
  rfConfig.txRxCLKPin = txRxCLKPin;
  pinMode(txRxCLKPin, INPUT);
}

void Rf7021::setCEPin(byte cePin) {
  rfConfig.cePin = cePin;
  pinMode(cePin, OUTPUT);
}

void Rf7021::setSREADPin(byte sreadPin) {
  rfConfig.sreadPin = sreadPin;
  pinMode(sreadPin, INPUT);
}

void Rf7021::setSLEPin(byte slePin) {
  rfConfig.slePin = slePin;
  pinMode(slePin, OUTPUT);
}

void Rf7021::setSDATAPin(byte sdataPin) {
  rfConfig.sdataPin = sdataPin;
  pinMode(sdataPin, OUTPUT);
}

void Rf7021::setSCLKPin(byte sclkPin) {
  rfConfig.sclkPin = sclkPin;
  pinMode(sclkPin, OUTPUT);
}

void Rf7021::setXtalFrequency(uint32_t xtalFrequency) {
  rfConfig.xtalFrequency = xtalFrequency;
}

void Rf7021::setXtalBias(byte xtalBias) {
  rfConfig.xtalBias = xtalBias;
}

void Rf7021::setCpCurrent(byte cpCurrent) {
  rfConfig.cpCurrent = cpCurrent;
}

void Rf7021::setHasExternalInductor(byte hasExternalInductor) {
  rfConfig.hasExternalInductor = hasExternalInductor;
}

void Rf7021::setmModulationScheme(byte modulationScheme) {
  rfConfig.modulationScheme = modulationScheme;
}

void Rf7021::setPowerAmplifierEnabled(byte powerAmplifierEnabled) {
  rfConfig.powerAmplifierEnabled = powerAmplifierEnabled;
}

void Rf7021::setPowerAmplifierRamping(byte powerAmplifierRamping) {
  rfConfig.powerAmplifierRamping = powerAmplifierRamping;
}

void Rf7021::setPowerAmplifierBias(byte powerAmplifierBias) {
  rfConfig.powerAmplifierBias = powerAmplifierBias;
}

void Rf7021::setPowerAmplifierPower(byte powerAmplifierPower) {
  rfConfig.powerAmplifierPower = powerAmplifierPower;
}

void Rf7021::setDataInvertType(byte dataInvertType) {
  rfConfig.dataInvertType = dataInvertType;
}

void Rf7021::setRCosineAlpha(byte rCosineAlpha) {
  rfConfig.rCosineAlpha = rCosineAlpha;
}

void Rf7021::setDataRate(word dataRate) {
  rfConfig.dataRate = dataRate;
}

void Rf7021::setFrequency(uint32_t frequency) {
  rfConfig.frequency = frequency;
}

void Rf7021::setFrequencyDeviation(word frequencyDeviation) {
  rfConfig.frequencyDeviation = frequencyDeviation;
}

void Rf7021::setDataInvertEnabled(byte dataInvertEnabled) {
  rfConfig.dataInvertEnabled = dataInvertEnabled;
}

void Rf7021::writeReg(RfReg *reg) {  
  digitalWrite(rfConfig.slePin, LOW);
  digitalWrite(rfConfig.sclkPin, LOW);

  for (byte i = 4; i > 0; i--) {
    byte buf = reg->dataBytes[i - 1];
    for (byte j = 8; j > 0; j--) {
      digitalWrite(rfConfig.sclkPin, LOW);
      digitalWrite(rfConfig.sdataPin, (buf & 0x80) ? HIGH : LOW);
      digitalWrite(rfConfig.sclkPin, HIGH);
      buf <<= 1;
    }
   digitalWrite(rfConfig.sclkPin, LOW);
  }

  digitalWrite(rfConfig.slePin, HIGH);
  delay(1);
  digitalWrite(rfConfig.sdataPin, LOW);
  digitalWrite(rfConfig.slePin, LOW);
}

Rf7021::RfReg Rf7021::readReg(word readbackConfig) {
  RfReg reg;
  reg.data = ((readbackConfig & 0x1F) << 4);

  reg.data |= 7;
  writeReg(&reg);
  reg.data = 0;
  
  digitalWrite(rfConfig.sdataPin, LOW);
  digitalWrite(rfConfig.sclkPin, LOW);
  digitalWrite(rfConfig.slePin, HIGH);

  // Skip first DB16 byte
  digitalWrite(rfConfig.sclkPin, HIGH);
  digitalWrite(rfConfig.sclkPin, LOW);
  
  byte buf = 0;
  for (byte i = 2; i > 0; i--) {
    for (byte j = 8; j > 0; j--) {
      digitalWrite(rfConfig.sclkPin, HIGH);
      buf <<= 1;
      if (digitalRead(rfConfig.sreadPin)) {
        buf |= 1;
      }
      digitalWrite(rfConfig.sclkPin, LOW);
    }
    reg.dataBytes[i - 1] = buf;
  }

  digitalWrite(rfConfig.sclkPin, HIGH);
  digitalWrite(rfConfig.slePin, LOW);
  digitalWrite(rfConfig.sclkPin, LOW);

  return reg;
}

word Rf7021::getSiliconRev() {
  powerOn();
  word value = readReg(0x1C).dataLower;
  powerOff();
  return value;
}

float Rf7021::getTemp() {
  powerOn();
  RfReg reg;
  reg.data = 8;
  reg.data |= 1 << 8;
  writeReg(&reg);
  reg = readReg(0x16);
  powerOff();
  return 469.5 - (7.2 * (reg.dataBytes[0] & 0x7F));
}

float Rf7021::getVoltage() {
  powerOn();
  RfReg reg;
  reg.data = 8;
  reg.data |= 1 << 8;
  writeReg(&reg);
  reg = readReg(0x15);
  powerOff();
  return (reg.dataBytes[0] & 0x7F) / 21.1;
}

void Rf7021::powerOn() {
  digitalWrite(rfConfig.cePin, HIGH);
  delay(1);

  // Prepare and push Register 1
  rfConfig.R1.addressBits = 1;
  rfConfig.R1.rCounter = 1;
  rfConfig.R1.clockoutDivide = 0;
  rfConfig.R1.xtalDoubler = 0;
  rfConfig.R1.xoscEnable = 1;
  rfConfig.R1.xtalBias = rfConfig.xtalBias;
  rfConfig.R1.cpCurrent = rfConfig.cpCurrent;
  rfConfig.R1.vcoEnable = 1;
  if (rfConfig.frequency >= 900000000UL && rfConfig.frequency <= 950000000UL) {
    // 900 - 950 MHz internal VCO
    rfConfig.R1.rfDivideBy2 = 0;
    rfConfig.R1.vcoBias = 8;
    rfConfig.R1.vcoAdjust = 3;
    rfConfig.R1.vcoInductor = 0; 
  } else if (rfConfig.frequency >= 800000000UL && rfConfig.frequency < 900000000UL) {
    // 862 - 900 MHz internal VCO
    rfConfig.R1.rfDivideBy2 = 0;
    rfConfig.R1.vcoBias = 8;
    rfConfig.R1.vcoAdjust = 0;
    rfConfig.R1.vcoInductor = 0; 
  } else if (rfConfig.frequency >= 450000000UL && rfConfig.frequency < 500000000UL) {
    // 450 - 500 MHz internal VCO
    rfConfig.R1.rfDivideBy2 = 1;
    rfConfig.R1.vcoBias = 8;
    rfConfig.R1.vcoAdjust = 3;
    rfConfig.R1.vcoInductor = 0; 
  } else if (rfConfig.frequency >= 400000000UL && rfConfig.frequency < 450000000UL) {
    // 400 - 450 MHz internal VCO
    rfConfig.R1.rfDivideBy2 = 1;
    rfConfig.R1.vcoBias = 8;
    rfConfig.R1.vcoAdjust = 0;
    rfConfig.R1.vcoInductor = 0;
  } else if (rfConfig.hasExternalInductor && rfConfig.frequency >= 500000000UL && rfConfig.frequency <= 650000000UL) {
      // 500 - 650 MHz external VCO
      rfConfig.R1.rfDivideBy2 = 0;
      rfConfig.R1.vcoBias = 4;
      rfConfig.R1.vcoInductor = 1; 
    } else if (rfConfig.hasExternalInductor && rfConfig.frequency >= 200000000UL && rfConfig.frequency < 400000000UL) {
      // 200 - 400 MHz external VCO
      rfConfig.R1.rfDivideBy2 = 0;
      rfConfig.R1.vcoBias = 3;
      rfConfig.R1.vcoInductor = 1; 
    } else if (rfConfig.hasExternalInductor && rfConfig.frequency >= 80000000UL && rfConfig.frequency < 200000000UL) {
      // 80 - 200 MHz external VCO
      rfConfig.R1.rfDivideBy2 = 1;
      rfConfig.R1.vcoBias = 2;
      rfConfig.R1.vcoInductor = 1; 
    }
  writeReg(&rfConfig.r1);

  // Prepare and push Register 15 (CLK_MUX = 0x0 to enable default mode)
  rfConfig.R15.addressBits = 15;
  rfConfig.R15.rxTestMode = 0;
  rfConfig.R15.txTestMode = 0;
  rfConfig.R15.sdTestMode = 0;
  rfConfig.R15.cpTestMode = 0;
  rfConfig.R15.clkMux = 0;
  rfConfig.R15.pllTestMode = 0;
  rfConfig.R15.analogTestMode = 0;
  rfConfig.R15.forceLdHigh = 0;
  rfConfig.R15.reg1Pd = 0;
  rfConfig.R15.calOverride = 0;
  writeReg(&rfConfig.r15);

  // Prepare and push Register 3
  rfConfig.R3.addressBits = 3;
  // Find best BBOS divider
  for (byte bbosDiv = 0; bbosDiv < 4; bbosDiv++) {
    uint32_t bbosClk = round(rfConfig.xtalFrequency / (2 << (bbosDiv + 1)));
    if (bbosClk >= 1000000UL && bbosClk <= 2000000UL) {
      rfConfig.R3.bbosClkDivide = bbosDiv;
    }
  }
  // Find best DEMOD and CDR dividers to achieve required data rate
  double rateDeltaMin = rfConfig.dataRate;
  for (byte dem = 1; dem < 15; dem++) {
    byte cdr = round(rfConfig.xtalFrequency / (dem * rfConfig.dataRate * 32.0));
    if (cdr == 0) continue;
    word realRate = round(rfConfig.xtalFrequency / (dem * cdr * 32.0));
    double delta = abs(realRate - rfConfig.dataRate);
    if (delta < rateDeltaMin) {
      rfConfig.R3.demClkDivide = dem;
      rfConfig.R3.cdrClkDivide = cdr;
      rateDeltaMin = delta;
    }
  }
  rfConfig.R3.seqClkDivide = round(rfConfig.xtalFrequency / 100000.0);
  rfConfig.R3.agcClkDivide = round((rfConfig.R3.seqClkDivide * rfConfig.xtalFrequency) / 10000.0);
  writeReg(&rfConfig.r3);

  // Prepare and push Register 0
  rfConfig.R0.addressBits = 0;
  double n = (rfConfig.frequency * rfConfig.R1.rCounter / (rfConfig.xtalFrequency * (rfConfig.R1.rfDivideBy2 ? 0.5 : 1.0)));
  uint32_t nInt = floor(n);
  uint32_t nFrac = round((n - floor(n)) * 32768);
  rfConfig.R0.fractionalN = nFrac;
  rfConfig.R0.integerN = nInt;
  rfConfig.R0.rxMode = 0;
  rfConfig.R0.uartMode = 0;
  rfConfig.R0.muxOut = 0;
  writeReg(&rfConfig.r0);

  // Prepare and push Register 2
  rfConfig.R2.addressBits = 2;
  rfConfig.R2.modulationScheme = rfConfig.modulationScheme;
  rfConfig.R2.paEnable = rfConfig.powerAmplifierEnabled;
  rfConfig.R2.paRamp = rfConfig.powerAmplifierRamping;
  rfConfig.R2.paBias = rfConfig.powerAmplifierBias;
  rfConfig.R2.powerAmplifier = rfConfig.powerAmplifierPower;
  rfConfig.R2.txFrequencyDeviation = round(rfConfig.frequencyDeviation * 65536.0 * (rfConfig.R1.rfDivideBy2 ? 2.0 : 1.0) / ((double) rfConfig.xtalFrequency / rfConfig.R1.rCounter));
  rfConfig.R2.txDataInvert = rfConfig.dataInvertEnabled ? rfConfig.dataInvertType : 0;
  rfConfig.R2.rcosineAlpha = rfConfig.rCosineAlpha;
  writeReg(&rfConfig.r2);
}

void Rf7021::powerOff() {
  digitalWrite(rfConfig.cePin, LOW);
  delay(1);
}

void Rf7021::txTest(byte testMode, word duration) {
  powerOn();
  
  rfConfig.R15.txTestMode = testMode;
  writeReg(&rfConfig.r15);
  
  delay(duration);

  powerOff();
}

void Rf7021::sendMessage(byte *message, word messageLength) {
  powerOn();

  byte bitCounter = 0;
  word byteCounter = 0;
  byte currentByte = message[0];

  while (byteCounter < messageLength) {
    // Wait LOW on TxRxCLK
    while (digitalRead(rfConfig.txRxCLKPin));
    
    // Push bit
    if (currentByte & 0x80) {
      digitalWrite(rfConfig.txRxDataPin, HIGH);
    } else {
      digitalWrite(rfConfig.txRxDataPin, LOW);
    }

    // Switch to next bit
    bitCounter++;
    if (bitCounter >= 8) {
      bitCounter = 0;
      byteCounter++;
      currentByte = message[byteCounter];
    } else {
      currentByte <<= 1;
    }
    
    // Wait HIGH on TxRxCLK
    while (!digitalRead(rfConfig.txRxCLKPin));
  }

  powerOff();
}
