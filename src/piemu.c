///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
///////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "debugger.h"
#include "disassemble.h"
#include "memory.h"

enum { START_ADDR = 0x8000 };

// Simulate the Raspberry Pi being powered up
void power_on() {
	load_memory_from_file("kernel.img", START_ADDR);
	set_program_counter(START_ADDR + 8);						// 2 instruction pipeline.  PC is 8 bytes greater than currently executing instruction.
	run();
}

int main(int argc, char **argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "-d") == 0) {
			int size_in_words = load_memory_from_file("kernel.img", START_ADDR);

			for (int i = 0; i < size_in_words; i++) {
				printf("%s\n", disassemble(START_ADDR + i * 4));
			}
		} else {
			fprintf(stderr, "piemu: illegal option %s\n", argv[1]);
			fprintf(stderr, "usage: piemu [-d]\n\n");
		}
	} else {
		power_on();
	}
}