///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
// Represents the BCM2835.
//
///////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "error.h"
#include "gpio.h"

// GPIO register start and end addresses
enum {
	FUNCTION_SELECT_START = 0x20200000,
	FUNCTION_SELECT_END   = 0x20200014,

	PIN_OUTPUT_SET_START = 0x2020001c,
	PIN_OUTPUT_SET_END   = 0x20200020,

	PIN_OUTPUT_CLEAR_START = 0x20200028,
	PIN_OUTPUT_CLEAR_END   = 0x2020002c,

	NUM_GPIO_LINES                = 54,
	NUM_FUNCTION_SELECT_REGISTERS = 6
};

// Alternate functions
typedef enum {
	FUNCTION_SELECT_INPUT                = 0,
	FUNCTION_SELECT_OUTPUT               = 1,
	FUNCTION_SELECT_ALTERNATE_FUNCTION_0 = 4,
	FUNCTION_SELECT_ALTERNATE_FUNCTION_1 = 5,
	FUNCTION_SELECT_ALTERNATE_FUNCTION_2 = 6,
	FUNCTION_SELECT_ALTERNATE_FUNCTION_3 = 7,
	FUNCTION_SELECT_ALTERNATE_FUNCTION_4 = 3,
	FUNCTION_SELECT_ALTERNATE_FUNCTION_5 = 2
} function_select_t;

function_select_t function_select[NUM_GPIO_LINES];
bool pin_set[NUM_GPIO_LINES];

uint32_t gpio_read_word(uint32_t addr) {
    not_implemented(__func__, "Read from 0x%08x", addr);
    assert(0);
}

void gpio_write_word(uint32_t addr, uint32_t value) {
    assert(addr % 4 == 0);

    if (FUNCTION_SELECT_START <= addr && addr <= FUNCTION_SELECT_END) {
        int base = (addr - FUNCTION_SELECT_START) / 4 * 10;         // We are dealing with words so divide the address by the word length

        for (int i = 0; i < 10; i++) {
            function_select_t function = ((value >> (i * 3)) & 7);     // Each function is 3 bits
            function_select[base + i] = function;

            if (base + i == NUM_GPIO_LINES - 1)
                break;
        }
    } else if (PIN_OUTPUT_SET_START <= addr && addr <= PIN_OUTPUT_SET_END) {
        int base = (addr - PIN_OUTPUT_SET_START) / 4 * 32;

        for (int i = 0; i < 32; i++) {
            if (((value >> i) & 1) == 1) {
                // When pin 16 is set and the function is output this switches the OK LED off
                if (function_select[base + i] == FUNCTION_SELECT_OUTPUT)
                    printf("*** OK LED: OFF ***\n");

                pin_set[base + 1] = true;
            }

            if (base + i == NUM_GPIO_LINES - 1)
                break;
        }
    } else if (PIN_OUTPUT_CLEAR_START <= addr && addr <= PIN_OUTPUT_CLEAR_END) {
        int base = (addr - PIN_OUTPUT_CLEAR_START) / 4 * 32;

        for (int i = 0; i < 32; i++) {
            if (((value >> i) & 1) == 1) {
                // When pin 16 is cleared and the function is output this switches the OK LED on
                if (function_select[base + i] == FUNCTION_SELECT_OUTPUT)
                    printf("*** OK LED: ON ***\n");

                pin_set[base + 1] = false;
            }

            if (base + i == NUM_GPIO_LINES - 1)
                break;
        }
    } else
        not_implemented(__func__, "Write to 0x%08x with value 0x%08x", addr, value);
}
