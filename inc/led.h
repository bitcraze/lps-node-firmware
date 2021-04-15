#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

#define N_LEDS 3

typedef enum {
  ledRanging=0,
  ledSync,
  ledMode
} led_e;

void ledInit(void);

void ledOn(led_e led);

void ledOff(led_e led);

void ledBlink(led_e led, bool oneshot);

void ledTick();

#endif /* LED_H_ */
