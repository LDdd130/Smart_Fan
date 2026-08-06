#ifndef SYSTEM_TIMER_H
#define SYSTEM_TIMER_H

#include <stdint.h>

void system_timer_init(void);
void system_timer_tick(uint16_t elapsed_ms);
uint32_t system_millis_get(void);
uint8_t system_millis_elapsed(uint32_t *last_ms, uint16_t interval_ms);

#endif