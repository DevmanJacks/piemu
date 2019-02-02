#ifndef __MEMORY_H
#define __MEMORY_H

#include <stdint.h>

// Public functions
extern void load_memory_from_file(char *filename, uint32_t addr);

extern uint32_t read_word(uint32_t addr);
extern void write_word(uint32_t addr, uint32_t value);

#endif