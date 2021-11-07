/*
 * Simple library for using the RF7021SE (ADF7021) module to transfer binary data
 *
 * RF7021SE module pinout:
 * 
 * | VCC            | PAC (not used) |
 * | CE             | SLE            |
 * | SDATA          | SREAD          |
 * | NC (not used)  | SCLK           |
 * | TxRxData       | SWD (not used) |
 * | MUX (not used) | TxRxCLK        |
 * | GND            | GND            |
 *
 * Copyright (c) 2021, SinuX. All rights reserved.
 * 
 * This library is distributed "as is" without any warranty.
 * See MIT License for more details.
 */

#ifndef _RF7021_H_
#define _RF7021_H_

#include "Arduino.h"

class Rf7021 {

  public:
    Rf7021();

    // Wiring
    void setTxRxDataPin(byte txRxDataPin);
    void setTxRxCLKPin(byte txRxCLKPin);
    void setCEPin(byte cePin);
    void setSREADPin(byte sreadPin);
    void setSLEPin(byte slePin);
    void setSDATAPin(byte sdataPin);
    void setSCLKPin(byte sclkPin);

    // Readback
    word getSiliconRev();
    float getTemp();
    float getVoltage();

    // Setup
    void setXtalFrequency(uint32_t xtalFrequency);
    void setXtalBias(byte xtalBias);
    void setCpCurrent(byte cpCurrent);
    void setHasExternalInductor(byte hasExternalInductor);
    void setmModulationScheme(byte modulationScheme);
    void setPowerAmplifierEnabled(byte powerAmplifierEnabled);
    void setPowerAmplifierRamping(byte powerAmplifierRamping);
    void setPowerAmplifierBias(byte powerAmplifierBias);
    void setPowerAmplifierPower(byte powerAmplifierPower);
    void setDataInvertType(byte dataInvertType);
    void setRCosineAlpha(byte rCosineAlpha);
    void setDataRate(word dataRate);
    void setFrequency(uint32_t frequency);
    void setFrequencyDeviation(word frequencyDev);
    void setDataInvertEnabled(byte dataInvertEnabled);

    // Transmission
    void txTest(byte testMode, word duration);
    void sendMessage(byte *message, word messageLength);

  private:
    typedef union {
      uint32_t data;
      word dataLower;
      word dataUpper;
      byte dataBytes[4];
    } RfReg;

    typedef struct {
      // Wiring
      byte txRxDataPin, txRxCLKPin;
      byte cePin, sreadPin, slePin, sdataPin, sclkPin;

      // Core settings
      uint32_t xtalFrequency;
      byte xtalBias;
      byte cpCurrent;
      byte hasExternalInductor;

      // Transmission settings
      byte modulationScheme;
      byte powerAmplifierEnabled;
      byte powerAmplifierRamping;
      byte powerAmplifierBias;
      byte powerAmplifierPower;
      byte dataInvertType;
      byte rCosineAlpha;
      word dataRate;
      uint32_t frequency;
      word frequencyDeviation;
      byte dataInvertEnabled;
      
      union {
        RfReg r0;
        struct {
          byte addressBits : 4;
          word fractionalN : 15;
          word integerN : 8;
          byte rxMode : 1;
          byte uartMode : 1;
          byte muxOut : 3;
        } R0;
      };
      union {
        RfReg r1;
        struct {
          byte addressBits : 4;
          byte rCounter : 3;
          byte clockoutDivide : 4;
          byte xtalDoubler : 1;
          byte xoscEnable : 1;
          byte xtalBias : 2;
          byte cpCurrent : 2;
          byte vcoEnable : 1;
          byte rfDivideBy2 : 1;
          byte vcoBias : 4;
          byte vcoAdjust : 2;
          byte vcoInductor : 1;
        } R1;
      };
      union {
        RfReg r2;
        struct {
          byte addressBits : 4;
          byte modulationScheme : 3;
          byte paEnable : 1;
          byte paRamp : 3;
          byte paBias : 2;
          byte powerAmplifier : 6;
          word txFrequencyDeviation : 9;
          byte txDataInvert : 2;
          byte rcosineAlpha : 1;
        } R2;
      };
      union {
        RfReg r3;
        struct {
          byte addressBits : 4;
          byte bbosClkDivide: 2;
          byte demClkDivide : 4;
          byte cdrClkDivide : 8;
          byte seqClkDivide : 8;
          byte agcClkDivide : 6;
        } R3;
      };
      union {
        RfReg r15;
        struct {
          byte addressBits : 4;
          byte rxTestMode : 4;
          byte txTestMode : 3;
          byte sdTestMode : 3;
          byte cpTestMode : 3;
          byte clkMux : 3;
          byte pllTestMode : 4;
          byte analogTestMode : 4;
          byte forceLdHigh : 1;
          byte reg1Pd : 1;
          byte calOverride : 2;
        } R15;
      };
    } RfConfig;

    RfConfig rfConfig;

    void writeReg(RfReg *reg);
    RfReg readReg(word readbackConfig);

    void powerOn();
    void powerOff();
};


#endif /* _RF7021_H_ */
