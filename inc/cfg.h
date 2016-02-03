#ifndef __CFG_H__
#define __CFG_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  cfgAddress = 0,
  cfgMode,
  cfgAnchorlist
} ConfigField;

typedef enum {
  modeAnchor = 0,
  modeTag,
  modeSniffer
} CfgMode;

void cfgInit();

bool cfgReset(void);

bool cfgFieldSize(ConfigField field, uint8_t * size);

bool cfgReadU8(ConfigField field, uint8_t * value);

bool cfgWriteU8(ConfigField field, uint8_t data);

bool cfgReadU8list(ConfigField field, uint8_t list[], uint8_t lenth);

bool cfgWriteU8list(ConfigField field, uint8_t list[], uint8_t lenth);

#endif
