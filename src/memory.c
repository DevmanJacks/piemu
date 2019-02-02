///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
///////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "gpio.h"
#include "memory.h"

enum { MEMORY_SIZE_WORDS = 1024 * 16 };

static uint32_t memory[MEMORY_SIZE_WORDS];

enum {
    GPIO_START = 0x20200000,
    GPIO_END   = 0x202000b0
};

uint32_t read_word(uint32_t addr) {
    if (GPIO_START <= addr && addr <= GPIO_END)
        return gpio_read_word(addr);

    assert(addr / 4 < MEMORY_SIZE_WORDS);

    if ((addr & 3) == 0) {
        return memory[addr >> 2];
    }

    not_implemented(__func__, "Unaligned read word");
    assert(0);
}

void write_word(uint32_t addr, uint32_t value) {
    if (GPIO_START <= addr && addr <= GPIO_END)
        return gpio_write_word(addr, value);

    assert(addr / 4 < MEMORY_SIZE_WORDS);

    if ((addr & 3) == 0) {
        memory[addr >> 2] = value;
        return;
    }

    not_implemented(__func__, "Unaligned write word");
    assert(0);
}

void load_memory_from_file(char *filename, uint32_t addr) {
	FILE *f = fopen(filename, "rb");

	if (f == NULL) { 
		perror(filename);
		exit(2);
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size != fread(memory + addr / 4, sizeof(uint8_t), size, f)) {
		fprintf(stderr, "Problem reading reading file '%s'.", filename);
        exit(2);
	}

	fclose(f);
}