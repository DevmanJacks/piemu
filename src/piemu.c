///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
///////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
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
	power_on();
}