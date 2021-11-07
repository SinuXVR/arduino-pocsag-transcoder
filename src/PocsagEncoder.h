/*
 * Simple yet fully functional POCSAG encoder implementation capable
 * of generating any kind of message (tone, numeric or alphanumeric)
 *
 * Alphanumeric messages can be encoded using one of 4 encodings
 *
 * Copyright (c) 2021, SinuX. All rights reserved.
 * 
 * This library is distributed "as is" without any warranty.
 * See MIT License for more details.
 */

#ifndef _POCSAGENCODER_H_
#define _POCSAGENCODER_H_

#define PCSG_PREAMBLE_LENGTH 128  // Preamble length in bytes (must be >= 72)
#define PCSG_BATCHES_COUNT 5      // Batches count (5 batches can handle 185 to 225 alphanumeric symbols)
#define PCSG_MESSAGE_LENGTH PCSG_PREAMBLE_LENGTH + 68 * PCSG_BATCHES_COUNT  // Max message length in bytes including preamble and all batches
#define PCSG_EOT_CHAR 0x04        // Alphanumeric message text termination character code

#include "Arduino.h"

class PocsagEncoder {

  public:
    PocsagEncoder();

    void setCapCode(uint32_t capCode);
    void setSource(byte source);
    void setEncodingId(byte encodingId);
    void setPrintMessage(byte printMessage);

    typedef union {
      struct {
        byte parityBits : 1;
        word crcBits : 10;
        uint32_t payloadBits : 20; // 18 address + 2 source bits or 20 message bits
        byte messageType : 1;
      };
      uint32_t data;
    } PocsagCW;

    typedef struct {
      PocsagCW codeWords[2];
    } PocsagFrame;

    typedef struct {
      uint32_t fsc;
      PocsagFrame frames[8];
    } PocsagBatch;

    typedef struct {
      word messageLength;
      union {
        struct {
          byte preamble[PCSG_PREAMBLE_LENGTH];
          PocsagBatch batches[PCSG_BATCHES_COUNT];
        };
        byte dataBytes[PCSG_MESSAGE_LENGTH];
      };
    } PocsagMessage;

    PocsagMessage encodeNumeric(String messageText);
    PocsagMessage encodeAlphanumeric(String messageText);
    PocsagMessage encodeTone();

  private:
    uint32_t capCode;
    byte source;
    byte encodingId;
    byte printMessage;

    byte getPocsagNumericCharacter(byte asciiChar);
    byte getPocsagAlphanumericCharacter(word utf8Char, byte encodingId);

    struct {
      word offset;
      byte length;
    } PCSG_ENCODINGS[4] = {
      {48,  63},  // 0 - Latin (Motorola Advisor)
      {111, 75},  // 1 - Latin/Cyrillic (Motorola Advisor Linguist, Scriptor LX2 Linguist, NEC Maxima, Optima, Truly Supervisor)
      {186, 65},  // 2 - Latin/Cyrillic (Philips PRG2220, PRG2310)
      {251, 67}   // 3 - Cyrillic (Motorola Advisor, Scriptor LX1)
    };
};

static const byte PCSG_ALPHABET[318][2] PROGMEM = {
      // Numeric
      {'0', 0x0}, {'1', 0x1}, {'2', 0x2}, {'3', 0x3}, {'4', 0x4}, {'5', 0x5}, {'6', 0x6}, {'7', 0x7}, {'8', 0x8}, {'9', 0x9}, {'*', 0xA}, {'U', 0xB}, {' ', 0xC}, {'-', 0xD}, {')', 0xE}, {'(', 0xF},
      
      // Alphanumeric common
      {' ', 0x20}, {'!', 0x21}, {'"', 0x22}, {'#', 0x23}, {'$', 0x24}, {'%', 0x25}, {'&', 0x26}, {'\'', 0x27}, {'(', 0x28}, {')', 0x29}, {'*', 0x2A}, {'+', 0x2B}, {',', 0x2C}, {'-', 0x2D}, {'.', 0x2E}, {'/', 0x2F},
      {'0', 0x30}, {'1', 0x31}, {'2', 0x32}, {'3', 0x33}, {'4', 0x34}, {'5', 0x35}, {'6', 0x36}, {'7',  0x37}, {'8', 0x38}, {'9', 0x39}, {':', 0x3A}, {';', 0x3B}, {'<', 0x3C}, {'=', 0x3D}, {'>', 0x3E}, {'?', 0x3F},
      
      // 0 - Alphanumeric Latin (Motorola Advisor)
      {'@', 0x40}, {'A', 0x41}, {'B', 0x42}, {'C', 0x43}, {'D', 0x44}, {'E', 0x45}, {'F', 0x46}, {'G', 0x47}, {'H', 0x48}, {'I', 0x49}, {'J', 0x4A}, {'K', 0x4B}, {'L',  0x4C}, {'M', 0x4D}, {'N', 0x4E}, {'O', 0x4F},
      {'P', 0x50}, {'Q', 0x51}, {'R', 0x52}, {'S', 0x53}, {'T', 0x54}, {'U', 0x55}, {'V', 0x56}, {'W', 0x57}, {'X', 0x58}, {'Y', 0x59}, {'Z', 0x5A}, {'[', 0x5B}, {'\\', 0x5C}, {']', 0x5D}, {'^', 0x5E}, {'_', 0x5F},
      {'`', 0x60}, {'a', 0x61}, {'b', 0x62}, {'c', 0x63}, {'d', 0x64}, {'e', 0x65}, {'f', 0x66}, {'g', 0x67}, {'h', 0x68}, {'i', 0x69}, {'j', 0x6A}, {'k', 0x6B}, {'l',  0x6C}, {'m', 0x6D}, {'n', 0x6E}, {'o', 0x6F},
      {'p', 0x70}, {'q', 0x71}, {'r', 0x72}, {'s', 0x73}, {'t', 0x74}, {'u', 0x75}, {'v', 0x76}, {'w', 0x77}, {'x', 0x78}, {'y', 0x79}, {'z', 0x7A}, {'{', 0x7B}, {'|',  0x7C}, {'}', 0x7D}, {'~', 0x7E},

      // 1 - Alphanumeric Latin/Cyrillic (Motorola Advisor Linguist, Scriptor LX2 Linguist, NEC Maxima, Optima, Truly Supervisor)
      {'@', 0x40}, {'A', 0x41}, {192, 0x41}, {'B', 0x42}, {194, 0x42}, {'C', 0x43}, {209, 0x43}, {'D', 0x44}, {'E', 0x45}, {197, 0x45}, {'F', 0x46}, {'G', 0x47}, {'H', 0x48}, {205, 0x48}, {'I', 0x49}, {'J',  0x4A}, {'K', 0x4B}, {202, 0x4B}, {'L', 0x4C}, {'M', 0x4D}, {204, 0x4D}, {'N', 0x4E}, {'O', 0x4F}, {206, 0x4F},
      {'P', 0x50}, {208, 0x50}, {'Q', 0x51}, {'R', 0x52}, {'S', 0x53}, {'T', 0x54}, {210, 0x54}, {'U', 0x55}, {'V', 0x56}, {'W', 0x57}, {'X', 0x58}, {213, 0x58}, {'Y', 0x59}, {'Z', 0x5A}, {'[', 0x5B}, {'\\', 0x5C}, {']', 0x5D}, {'^', 0x5E}, {'_', 0x5F},
      {'`', 0x60}, {193, 0x61}, {195, 0x62}, {129, 0x63}, {131, 0x63}, {196, 0x64}, {168, 0x65}, {198, 0x66}, {199, 0x67}, {200, 0x68}, {201, 0x69}, {203, 0x6A}, {207, 0x6B}, {211, 0x6C}, {212, 0x6D}, {214,  0x6E}, {215, 0x6F},
      {216, 0x70}, {217, 0x71}, {218, 0x72}, {219, 0x73}, {220, 0x74}, {221, 0x75}, {222, 0x76}, {223, 0x77}, {142, 0x78}, {170, 0x79}, {175, 0x7A}, {'{', 0x7B}, {'|', 0x7C}, {'}', 0x7D}, {'~', 0x7E},

      // 2 - Alphanumeric Latin/Cyrillic (Philips PRG2220, PRG2310)
      {'@', 0x40}, {'A', 0x41}, {'B', 0x42}, {'C', 0x43}, {'D', 0x44}, {'E', 0x45}, {'F', 0x46}, {'G', 0x47}, {'H', 0x48}, {'I', 0x49}, {'J', 0x4A}, {'K', 0x4B}, {'L',  0x4C}, {'M', 0x4D},  {'N', 0x4E}, {'O', 0x4F},
      {'P', 0x50}, {'Q', 0x51}, {'R', 0x52}, {'S', 0x53}, {'T', 0x54}, {'U', 0x55}, {'V', 0x56}, {'W', 0x57}, {'X', 0x58}, {'Y', 0x59}, {'Z', 0x5A}, {'[', 0x5B}, {'\\', 0x5C}, {']', 0x5D},  {'^', 0x5E}, {'_', 0x5F},
      {192, 0x60}, {193, 0x61}, {194, 0x62}, {195, 0x63}, {196, 0x64}, {197, 0x65}, {168, 0x65}, {198, 0x66}, {199, 0x67}, {200, 0x68}, {201, 0x69}, {202, 0x6A}, {203, 0x6B},  {204,  0x6C}, {205, 0x6D}, {206, 0x6E}, {207, 0x6F},
      {208, 0x70}, {209, 0x71}, {210, 0x72}, {211, 0x73}, {212, 0x74}, {213, 0x75}, {214, 0x76}, {215, 0x77}, {216, 0x78}, {217, 0x79}, {218, 0x7A}, {219, 0x7B}, {220,  0x7C}, {221, 0x7D},  {222, 0x7E}, {223, 0x7F},

      // 3 - Alphanumeric Cyrillic (Motorola Advisor, Scriptor LX1)
      {254, 0x40}, {224, 0x41}, {225, 0x42}, {246, 0x43}, {228, 0x44}, {229, 0x45}, {184, 0x45}, {244, 0x46}, {227, 0x47}, {245, 0x48}, {232, 0x49}, {233, 0x4A}, {234, 0x4B}, {235, 0x4C}, {236, 0x4D}, {237, 0x4E}, {238, 0x4F},
      {239, 0x50}, {255, 0x51}, {240, 0x52}, {241, 0x53}, {242, 0x54}, {243, 0x55}, {230, 0x56}, {226, 0x57}, {250, 0x58}, {252, 0x58}, {251, 0x59}, {231, 0x5A}, {248, 0x5B}, {253, 0x5C}, {249, 0x5D}, {247, 0x5E}, {'_', 0x5F},
      {222, 0x60}, {192, 0x61}, {193, 0x62}, {214, 0x63}, {196, 0x64}, {197, 0x65}, {168, 0x65}, {212, 0x66}, {195, 0x67}, {213, 0x68}, {200, 0x69}, {201, 0x6A}, {202, 0x6B}, {203, 0x6C}, {204, 0x6D}, {205, 0x6E}, {206, 0x6F},
      {207, 0x70}, {223, 0x71}, {208, 0x72}, {209, 0x73}, {210, 0x74}, {211, 0x75}, {198, 0x76}, {194, 0x77}, {218, 0x78}, {220, 0x78}, {219, 0x79}, {199, 0x7A}, {216, 0x7B}, {221, 0x7C}, {217, 0x7D}, {215, 0x7E}
    };

#endif /* _POCSAGENCODER_H_ */
