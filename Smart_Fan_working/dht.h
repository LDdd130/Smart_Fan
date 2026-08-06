#ifndef DHT_H
#define DHT_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "app_config.h"
#include "pinmap.h"

void DHT_SetInput(void);
uint8_t DHT_Read(uint8_t *temperature, uint8_t *humidity);

#endif