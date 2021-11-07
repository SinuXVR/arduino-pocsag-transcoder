#include "PocsagEncoder.h"

PocsagEncoder::PocsagEncoder() {
}

void PocsagEncoder::setCapCode(uint32_t capCode) {
  this->capCode = capCode;
}

void PocsagEncoder::setSource(byte source) {
  this->source = source;
}

void PocsagEncoder::setEncodingId(byte encodingId) {
  this->encodingId = encodingId;
}

void PocsagEncoder::setPrintMessage(byte printMessage) {
  this->printMessage = printMessage;
}

static word calculateBCH(uint32_t cw) {
  uint32_t generator = 0xED200000;
  uint32_t mask = 0x80000000;

  for (byte i = 0; i < 21; i++) {
    if (cw & mask) {
      cw ^= generator;
    }
    generator >>= 1;
    mask >>= 1;
  }
  
  return cw >> 1;
}

static byte calculateParity(uint32_t cw) {
  byte parity = 0;

  for (byte i = 0; i < 32; i++) {
    if (cw & 0x80000000) {
      parity ^= 1;
    }
    cw <<= 1;
  }

  return parity;
}

static void flipEndianess(uint32_t *value) {
  uint8_t *bytes = (uint8_t*) value;
  uint8_t tmp = bytes[0];
  bytes[0] = bytes[3];
  bytes[3] = tmp;
  tmp = bytes[1];
  bytes[1] = bytes[2];
  bytes[2] = tmp;
}

static void fillCW(PocsagEncoder::PocsagCW *codeWord, byte messageType, uint32_t payloadBits) {
  codeWord->data = 0;
  codeWord->messageType = messageType;
  codeWord->payloadBits = payloadBits;
  codeWord->crcBits = calculateBCH(codeWord->data);
  codeWord->parityBits = calculateParity(codeWord->data);
  flipEndianess(&codeWord->data);
}

static void fillAddressCW(PocsagEncoder::PocsagCW *codeWord, uint32_t capCode, byte source) {
  fillCW(codeWord, 0, ((capCode >> 3) << 2) | source);
}

static void fillMessageCW(PocsagEncoder::PocsagCW *codeWord, uint32_t messageBits) {
  fillCW(codeWord, 1, messageBits);
}

// Fill preamble bytes with 0xAA and frames with idle code words, fill address code word and return first frame number
static byte prepareMessage(PocsagEncoder::PocsagMessage *message, uint32_t capCode, byte source) {
  // Fill preamble
  for (byte i = 0; i < PCSG_PREAMBLE_LENGTH; i++) {
    message->preamble[i] = 0xAA;
  }
  // Fill frames with idle code words
  for (byte i = 0; i < PCSG_BATCHES_COUNT; i++) {
    message->batches[i].fsc = 0xD814D27C; // 0x7CD214D8
    for (byte j = 0; j < 8; j++) {
      message->batches[i].frames[j].codeWords[0].data = 0x97C1897A; // 0x7A89C197
      message->batches[i].frames[j].codeWords[1].data = 0x97C1897A; // 0x7A89C197
    }
  }

  // Determine first frame number
  byte currentFrame = capCode & 0x7;

  // Fill address code word
  fillAddressCW(&message->batches[0].frames[currentFrame].codeWords[0], capCode, source);

  return currentFrame;
}

// Convert 1-byte ASCII character to 5-bit Pocsag digit
byte PocsagEncoder::getPocsagNumericCharacter(byte asciiChar) {
  for (byte i = 0; i < 16; i++) {
    if (pgm_read_byte(&PCSG_ALPHABET[i][0]) == asciiChar) {
      return pgm_read_byte(&PCSG_ALPHABET[i][1]);
    }
  }

  return 0xC; // ' ' (space) - unknown digit
}

// Convert 2-byte Latin or Cyrillic UTF-8 character to 1-byte Windows-1251
static inline byte utf8ToWindows1251(word character, byte flipCase) {
  // Latin
  if (character <= 0x7E) {
    if (flipCase && character >= 0x41 && character <= 0x7A) {
      return character ^ (1 << 5);
    }
    return character;
  }
  
  // Cyrillic
  // Special cyrillic characters workaround
  switch(character) {
    case 0xD081: if (flipCase) return 184; else return 168; // Ё/ё
    case 0xD191: if (flipCase) return 168; else return 184; // ё/Ё
    case 0xD083: if (flipCase) return 131; else return 129; // Ѓ/ѓ
    case 0xD193: if (flipCase) return 129; else return 131; // ѓ/Ѓ
    case 0xD084: if (flipCase) return 186; else return 170; // Є/є
    case 0xD194: if (flipCase) return 170; else return 186; // є/Є
    case 0xD087: if (flipCase) return 191; else return 175; // Ї/ї
    case 0xD197: if (flipCase) return 175; else return 191; // ї/Ї
    case 0xD08B: if (flipCase) return 158; else return 142; // Ћ/ћ
    case 0xD19B: if (flipCase) return 142; else return 158; // ћ/Ћ
    default: break;
  }

  // Main cyrillic characters
  if (character >= 0xD090 && character <= 0xD18F) {
    if (character >= 0xD090 && character <= 0xD0BF) {
      // 'А' - 'п'
      character -= 0xCFD0;
    } else if (character >= 0xD180 && character <= 0xD18F) {
      // 'р' - 'я'
      character -= 0xD090;
    }
    if (flipCase) {
      return character ^ (1 << 5);
    }
    return character;
  }

  return 0x3F; // '?' - unknown symbol;
}

// Convert 2-byte Latin or Cyrillic UTF-8 character to 7-bit Pocsag character
byte PocsagEncoder::getPocsagAlphanumericCharacter(word utf8Char, byte encodingId) {
  byte winChar = utf8ToWindows1251(utf8Char, 0);
  if (winChar == 0x3F) {
    return 0x3F;
  }

  // Search in common section
  for (byte i = 16; i < 48; i++) {
    if (pgm_read_byte(&PCSG_ALPHABET[i][0]) == winChar) {
      return pgm_read_byte(&PCSG_ALPHABET[i][1]);
    }
  }
  
  // Search in current case
  for (word i = PCSG_ENCODINGS[encodingId].offset; i < PCSG_ENCODINGS[encodingId].offset + PCSG_ENCODINGS[encodingId].length; i++) {
    if (pgm_read_byte(&PCSG_ALPHABET[i][0]) == winChar) {
      return pgm_read_byte(&PCSG_ALPHABET[i][1]);
    }
  }

  // Search in flipped case:
  winChar = utf8ToWindows1251(utf8Char, 1);
  for (word i = PCSG_ENCODINGS[encodingId].offset; i < PCSG_ENCODINGS[encodingId].offset + PCSG_ENCODINGS[encodingId].length; i++) {
    if (pgm_read_byte(&PCSG_ALPHABET[i][0]) == winChar) {
      return pgm_read_byte(&PCSG_ALPHABET[i][1]);
    }
  }

  return 0x3F; // '?' - unknown symbol
}

static void printMsg(PocsagEncoder::PocsagMessage message) {    
  byte byteCounter = 0;
  byte wordCounter = 0;
  uint32_t byteAcc = 0;
  for (word i = PCSG_PREAMBLE_LENGTH; i < message.messageLength; i++) {
    byteAcc <<= 8;
    byteAcc |= message.dataBytes[i];
    if (++byteCounter >= 4) {
      Serial.print(byteAcc, HEX);
      if (++wordCounter % 8 == 1) {
        Serial.println();
      } else {
        Serial.print(" ");
        if (wordCounter >= 18) {
          wordCounter = 1;
          Serial.println();
        }
      }
      byteCounter = 0;
      byteAcc = 0;
    }
  }
}

static void incrementCounters(byte *currentBatch, byte *currentFrame, byte *currentCodeWord) {
  (*currentCodeWord)++;
  if (*currentCodeWord >= 2) {
    *currentCodeWord = 0;
    (*currentFrame)++;
    if (*currentFrame >= 8) {
      *currentFrame = 0;
      (*currentBatch)++;
    }
  }
}

static void pushPocsagChar(byte pocsagChar, byte charSize, uint32_t *messageBits, byte *messageBitsLength,
                           PocsagEncoder::PocsagMessage *message, byte *currentBatch, byte *currentFrame, byte *currentCodeWord) {
  for (byte b = 0; b < charSize; b++) {
    *messageBits |= pocsagChar & 1;
    (*messageBitsLength)++;

    // Push message bits to code word
    if (*messageBitsLength >= 20) {
      *messageBitsLength = 0;
      fillMessageCW(&message->batches[*currentBatch].frames[*currentFrame].codeWords[*currentCodeWord], *messageBits);
      incrementCounters(currentBatch, currentFrame, currentCodeWord);
    }

    *messageBits <<= 1;
    pocsagChar >>= 1;
  }
}

PocsagEncoder::PocsagMessage PocsagEncoder::encodeNumeric(String messageText) {
  PocsagMessage message;

  // Prepare message and fill address CW
  byte currentFrame = prepareMessage(&message, capCode, source);
  byte currentBatch = 0;
  byte currentCodeWord = 1;
  uint32_t messageBits = 0;
  byte messageBitsLength = 0;

  // Convert and push all characters to message code words
  for (word i = 0; i < messageText.length(); i++) {
    byte nextChar = messageText.charAt(i);

    // Skip leading space and non-printable symbols
    if ((i == 0 && nextChar == 0x20) || nextChar < 0x20) {
      continue;
    }

    // Translate to pocsag digit
    byte pocsagChar = getPocsagNumericCharacter(nextChar);
    
    // Push next digit to message bits
    pushPocsagChar(pocsagChar, 4, &messageBits, &messageBitsLength, &message, &currentBatch, &currentFrame, &currentCodeWord);
  }

  // Fill empty space of last message codeword with 0xC character
  if (messageBitsLength > 0 && messageBitsLength < 20) {
    byte freeSpace = 20 - messageBitsLength;
    for (byte i = 0; i < freeSpace / 4; i++) {
      pushPocsagChar(0xC, 4, &messageBits, &messageBitsLength, &message, &currentBatch, &currentFrame, &currentCodeWord);
    }
  }
  
  message.messageLength = PCSG_PREAMBLE_LENGTH + 68 * (currentBatch + 1);

  if (printMessage) {
    printMsg(message);
  }
  
  return message;
}

PocsagEncoder::PocsagMessage PocsagEncoder::encodeAlphanumeric(String messageText) {
  PocsagMessage message;
  
  // Prepare message and fill address CW
  byte currentFrame = prepareMessage(&message, capCode, source);
  byte currentBatch = 0;
  byte currentCodeWord = 1;
  uint32_t messageBits = 0;
  byte messageBitsLength = 0;

  // Convert and push all characters to message code words
  for (word i = 0; i < messageText.length(); i++) {
    word nextChar = messageText.charAt(i);

    // Skip leading space and non-printable symbols
    if ((i == 0 && nextChar == 0x20) || nextChar < 0x20) {
      continue;
    }
    
    // 2-byte UTF-8 symbol - read second byte (workaround for Cyrillic characters)
    if (nextChar & 0x80) {
      i++;
      nextChar <<= 8;
      nextChar |= (byte) messageText.charAt(i);
    }

    // Translate to pocsag alphabet using current encoding
    byte pocsagChar = getPocsagAlphanumericCharacter(nextChar, encodingId);

    // Check if there is enough space to put next char in the last code word of the last batch
    // We must preserve at least 7 bits to put EOT/ETX character at the end of the message
    if (currentBatch == PCSG_BATCHES_COUNT - 1 && currentFrame == 7 && currentCodeWord == 1) {
      // Skip all remaining chars because they won't fit
      if (messageBitsLength >= 7) {
        break;
      }
    }
    
    // Push next character to message bits
    pushPocsagChar(pocsagChar, 7, &messageBits, &messageBitsLength, &message, &currentBatch, &currentFrame, &currentCodeWord);
  }

  // Push EOT symbol to the end of the message
  byte eotChar = PCSG_EOT_CHAR;
  pushPocsagChar(eotChar, 7, &messageBits, &messageBitsLength, &message, &currentBatch, &currentFrame, &currentCodeWord);

  // Fill last message code word
  if (messageBitsLength > 0 && messageBitsLength < 20) {
    // Try to push additional EOTs
    byte freeSpace = 20 - messageBitsLength;
    for (byte i = 0; i < freeSpace / 7; i++) {
      pushPocsagChar(eotChar, 7, &messageBits, &messageBitsLength, &message, &currentBatch, &currentFrame, &currentCodeWord);
    }
    // Trying to fill left empty space with EOT bits partially
    for (byte b = 0; b < 19 - messageBitsLength; b++) {
      messageBits |= eotChar & 1;
      messageBits <<= 1;
      eotChar >>= 1;
    }
    fillMessageCW(&message.batches[currentBatch].frames[currentFrame].codeWords[currentCodeWord], messageBits);
  }
  
  message.messageLength = PCSG_PREAMBLE_LENGTH + 68 * (currentBatch + 1);

  if (printMessage) {
    printMsg(message);
  }
  
  return message;
}

PocsagEncoder::PocsagMessage PocsagEncoder::encodeTone() {
  PocsagMessage message;
  
  // Fill address CW only
  prepareMessage(&message, capCode, source);
  message.messageLength = PCSG_PREAMBLE_LENGTH + 68;

  if (printMessage) {
    printMsg(message);
  }
  
  return message;
}
