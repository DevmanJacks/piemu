#ifndef __CPU_H
#define __CPU_H

#include <stdint.h>

// Named registers
typedef enum {
	sp = 13,
	lr = 14,
	pc = 15
} named_register_t;

static char *register_names[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc" };

// Instruction masks
enum {
    CONDITION_SHIFT = 28,
    CONDITION_MASK = 15,

	REGISTER_MASK = 15,

	DATA_PROCESSING_MASK           = 3 << 26,
	DATA_PROCESSING_IMMEDIATE_MASK = 1 << 25,
	DATA_PROCESSING_LSL_MASK       = 7<<25 | 7<<4,

	OPCODE_MASK    = 15,
	ROTATE_MASK    = 15,
	IMMEDIATE_MASK = 255,
	SHIFT_MASK     = 31,

	LOAD_STORE_WORD_OR_UNSIGNED_BYTE      = 1 << 26,
	LOAD_STORE_WORD_OR_UNSIGNED_BYTE_MASK = 3 << 26,

	BRANCH      = 5 << 25,
	BRANCH_MASK = 7 << 25
};

// Data processing opcodes
enum {
	OPCODE_SUB = 2,
	OPCODE_CMP = 10,
	OPCODE_MOV = 13
};

// Public functions
extern uint32_t program_counter();
extern void set_program_counter(uint32_t addr);

extern void print_cpsr();
extern void print_registers();
extern void step();

#endif