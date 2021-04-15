/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * LPS node firmware.
 *
 * Copyright 2016, Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stm32f0xx_hal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cfg.h"
#include "eeprom.h"
#include "uwb.h"

typedef struct {
  uint8_t *data;
} TlvArea;

// Temporary fix, just buffer the first 100 bytes
#define NUMBER_OF_BYTES_READ 100
#define MAGIC ((uint8_t) 0xBC)

#define SIZE_HEADER 5
#define SIZE_TAIL 1

static uint8_t buffer[NUMBER_OF_BYTES_READ];

static TlvArea tlv;

typedef struct {
  uint8_t magic;
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint16_t tlvSize;
} __attribute__((packed)) CfgHeader;

static CfgHeader * cfgHeader = (CfgHeader*) buffer;

static int tlvFindType(TlvArea *tlv, ConfigField type) {
  uint16_t pos = 0;

  while (pos < cfgHeader->tlvSize) {
    if (tlv->data[pos] == type) {
      return pos;
    } else {
      pos += tlv->data[pos+1]+2;
    }
  }
  return -1;
}

bool deckTlvHasElement(TlvArea *tlv, ConfigField type) {
  return tlvFindType(tlv, type) >= 0;
}

/* Reads all the data from the EEPROM */
static bool readData(void) {
  //int i;
  if (eepromRead(0, buffer, NUMBER_OF_BYTES_READ)) {
    /*printf("EEPROM: ");
    for (i = 0; i <NUMBER_OF_BYTES_READ; i++)
      printf("0x%02X ", buffer[i]);
    printf("\r\n");*/
    return true;
  } else {
    printf("CONFIG\t: Failed to read data from EEPROM!\r\n");
    return false;
  }
}

static void write_crc(void) {
  int i;
  uint8_t checksum = 0;

  for (i = 0; i < SIZE_HEADER + cfgHeader->tlvSize ; i++) {
    checksum += buffer[i];
  }
  buffer[SIZE_HEADER + cfgHeader->tlvSize] = checksum;
}

static bool check_crc(void) {
  int total_len = SIZE_HEADER + SIZE_TAIL + cfgHeader->tlvSize;
  uint8_t ref_checksum = buffer[total_len - SIZE_TAIL];
  int i;
  uint8_t checksum = 0;

  for (i = 0; i < SIZE_HEADER + cfgHeader->tlvSize; i++) {
    checksum += buffer[i];
  }
  if (checksum == ref_checksum) {
    return true;
  } else {
    printf("CONFIG\t: EEPROM configuration checksum not correct (0x%02X vs 0x%02X)!\r\n", ref_checksum, checksum);
    return false;
  }
}

static bool check_magic(void) {
  return (cfgHeader->magic == MAGIC);
}

static bool check_content(void) {
  if (check_magic() && cfgHeader->tlvSize < (NUMBER_OF_BYTES_READ - SIZE_HEADER -SIZE_TAIL)) {
    return check_crc();
  }
  printf("CONFIG\t: EEPROM magic not found!\r\n");
  return false;
}

static bool write_defaults(void) {
  uint8_t default_anchor_list[] = {1, 2, 3, 4, 5, 6};

  buffer[0] = MAGIC;
  buffer[1] = 1; // Major version
  buffer[2] = 0; // Minor version
  buffer[3] = 0; // Length of TLV
  buffer[4] = 0; // Length of TLV
  buffer[5] = buffer[0] + buffer[1];
  // Write the default address
  cfgWriteU8(cfgAddress, 0);
  cfgWriteU8(cfgMode, MODE_ANCHOR);
  cfgWriteU8list(cfgAnchorlist, default_anchor_list, sizeof(default_anchor_list));
  write_crc();
  if (!eepromWrite(0, buffer, 7))
    return false;
  HAL_Delay(10);
  if (readData()) {
    if (check_content()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void cfgInit(void) {
  if (readData()) {
    cfgHeader = (CfgHeader*) buffer;
    tlv.data = &buffer[SIZE_HEADER];
    if (check_content()) {
      printf("CONFIG\t: EEPROM configuration read and verified\r\n");
    } else {
      printf("CONFIG\t: Writing default EEPROM configuration\r\n");
      if (write_defaults()) {
      } else {
        printf("CONFIG\t: Error when writing default configuration\r\n");
      }
    }
  } else {
    printf("CONFIG\t: Could not read data from EEPROM\r\n");
  }
}

bool cfgReset(void) {
  uint8_t data = 0;
  bool ret = eepromWrite(0, &data, 1);
  HAL_Delay(10);
  return ret;
}

bool cfgReadU8(ConfigField field, uint8_t * value) {
  int pos = tlvFindType(&tlv, field);

  if (pos > -1) {
    *value = tlv.data[pos+2];
  }

  return (pos > -1);
}

bool cfgWriteU8(ConfigField field, uint8_t value) {
    int pos = tlvFindType(&tlv, field);

    if (pos > -1) {
      tlv.data[pos+2] = value;
    } else {
      // Add new field at the end of the tlv
      tlv.data[cfgHeader->tlvSize] = field;
      tlv.data[cfgHeader->tlvSize+1] = 1;
      tlv.data[cfgHeader->tlvSize+2] = value;
      cfgHeader->tlvSize += 3;
    }

    write_crc();
    eepromWrite(0, buffer, NUMBER_OF_BYTES_READ);
    HAL_Delay(10);
    readData();
    return true;
}

bool cfgReadU32(ConfigField field, uint32_t * value) {
  int pos = tlvFindType(&tlv, field);

  if (pos > -1) {
    memcpy(value, &tlv.data[pos+2], sizeof(uint32_t));
  }

  return (pos > -1);
}

bool cfgWriteU32(ConfigField field, uint32_t value) {
    int pos = tlvFindType(&tlv, field);

    if (pos > -1) {
      memcpy(&tlv.data[pos+2], &value, sizeof(uint32_t));
    } else {
      // Add new field at the end of the tlv
      tlv.data[cfgHeader->tlvSize] = field;
      tlv.data[cfgHeader->tlvSize+1] = sizeof(uint32_t);
      memcpy(&tlv.data[cfgHeader->tlvSize+2], &value, sizeof(uint32_t));
      cfgHeader->tlvSize += 2 + sizeof(uint32_t);
    }

    write_crc();
    eepromWrite(0, buffer, NUMBER_OF_BYTES_READ);
    HAL_Delay(10);
    readData();
    return true;
}

bool cfgReadU8list(ConfigField field, uint8_t list[], uint8_t length) {
  int pos = tlvFindType(&tlv, field);

  if (pos > -1) {
    memcpy(list, &tlv.data[pos+2], length);
  }

  return (pos > -1);
}

bool cfgFieldSize(ConfigField field, uint8_t * size) {
  int pos = tlvFindType(&tlv, field);

  if (pos > -1) {
    *size = tlv.data[pos+1];
  }

  return (pos > -1);
}

bool cfgWriteU8list(ConfigField field, uint8_t list[], uint8_t length) {
    int pos = tlvFindType(&tlv, field);

    if (pos > -1) {
      printf("Witing the list is not supported!!\r\n");
      //tlv.data[pos+2] = value;
      // TODO: The list can vary in length, we need to take care of that :-(
    } else {
      // Add new field at the end of the tlv
      tlv.data[cfgHeader->tlvSize] = field;
      tlv.data[cfgHeader->tlvSize+1] = length;
      memcpy(&tlv.data[cfgHeader->tlvSize+2], list, length);
      cfgHeader->tlvSize += 2 + length;
    }

    write_crc();
    eepromWrite(0, buffer, NUMBER_OF_BYTES_READ);
    HAL_Delay(10);
    readData();
    return true;
}

bool cfgReadFP32listLength(ConfigField field, uint8_t * size) {
  bool success = cfgFieldSize(field, size);
  *size /= 4;
  return success;
}

bool cfgReadFP32list(ConfigField field, float list[], uint8_t length) {
  int pos = tlvFindType(&tlv, field);

  if (pos > -1) {
    memcpy(list, &tlv.data[pos+2], length*sizeof(float));
  }

  return (pos > -1);
}

bool cfgWriteFP32list(ConfigField field, float list[], uint8_t length) {
    int pos = tlvFindType(&tlv, field);

    if (pos > -1) {
      uint8_t lengthInMemory=0;
      cfgReadFP32listLength(field, &lengthInMemory);
      if (lengthInMemory != length) {
        printf("Error: cannot write config list with different length\r\n");
        return false;
      }
      memcpy(&tlv.data[pos+2], list, length*sizeof(float));
    } else {
      // Add new field at the end of the tlv
      tlv.data[cfgHeader->tlvSize] = field;
      tlv.data[cfgHeader->tlvSize+1] = length*sizeof(float);
      memcpy(&tlv.data[cfgHeader->tlvSize+2], list, length*sizeof(float));
      cfgHeader->tlvSize += 2 + (length*sizeof(float));
    }

    write_crc();
    eepromWrite(0, buffer, NUMBER_OF_BYTES_READ);
    HAL_Delay(10);
    readData();
    return true;
}

static bool binaryMode = false;

void cfgSetBinaryMode(bool enable)
{
  binaryMode = enable;
}

bool cfgIsBinaryMode()
{
  return binaryMode;
}
