#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>

// Public functions
extern uint32_t gpio_read_word(uint32_t addr);
extern void gpio_write_word(uint32_t addr, uint32_t value);

#endif