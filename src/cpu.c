///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
// Represents a ARM1176JZF-S
//
///////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include "cpu.h"
#include "error.h"
#include "memory.h"

// Processor modes
typedef enum {
	MODE_USER       = 16,
	MODE_FIQ        = 17,
	MODE_IRQ        = 18,
	MODE_SUPERVISOR = 19,
	MODE_ABORT      = 23,
	MODE_UNDEFINED  = 27,
	MODE_SYSTEM     = 31
} processor_mode_t;

// ARM1176JZF-S starts off in system mode
static processor_mode_t mode = MODE_SYSTEM;

// Only 16 visible at any time.  Others are switched based on mode.
enum { NUM_REGISTERS = 32 };

uint32_t registers[NUM_REGISTERS];

// CPSR
static int n_flag = 0;
static int z_flag = 1;
static int c_flag = 0;
static int v_flag = 0;

static uint32_t read_register(int reg) {
	assert(0 <= reg && reg <= 15);

	if (mode == MODE_USER || mode == MODE_SYSTEM)
		return registers[reg];

	not_implemented(__func__, "Mode %d", mode);
	return 0;			// Can't get here
}

static void write_register(int reg, uint32_t value) {
	assert(0 <= reg && reg <= 15);

	if (mode == MODE_USER || mode == MODE_SYSTEM)
		registers[reg] = value;
	else
		not_implemented(__func__, "Mode %d", mode);
}

// Returns the current program counter
uint32_t program_counter() {
	return registers[pc];
}

// Sets the program counter to the specified address.
// NOTE: This should take into account the 8 byte pipeline.
void set_program_counter(uint32_t addr) {
	write_register(pc, addr);
}

// Returns the instruction at the current program counter minus the 8 byte pipeline.
uint32_t fetch_instruction() {
	return read_word(read_register(pc) - 8);		// 2 instruction pipeline
}

static void execute_branch(uint32_t instruction) {
	int l = instruction >> 24 & 1;
	int signedImmed24 = instruction & 0x00FFFFFF;

	uint32_t targetAddress = ((signedImmed24 << 8) >> 6) + read_register(pc) + 8;		// Add extra 2 words for pipeline

	if (l == 0)
		write_register(pc, targetAddress);
	else
		not_implemented(__func__, "Branch and link");
}

static void execute_data_processing(uint32_t instruction) {
	int i = instruction >> 25 & 1;
	int opcode = instruction >> 21 & OPCODE_MASK;
	int s = instruction >> 20 & 1;
	int rn = instruction >> 16 & REGISTER_MASK;
	int rd = instruction >> 12 & REGISTER_MASK;

	uint32_t operand;

	if (i == 1) {
		int rotate = (instruction >> 8 & ROTATE_MASK) << 1;
		int immediate = instruction & IMMEDIATE_MASK;
		operand = (immediate >> rotate) | (immediate << (32 - rotate));
		// TODO: Set carry out
	} else if ((instruction & DATA_PROCESSING_LSL_MASK) == 0) {
		int shift = instruction >> 7 & SHIFT_MASK;
		int rm = instruction & REGISTER_MASK;
		uint32_t rmValue = read_register(rm);
		uint32_t rnValue = read_register(rn);
		operand = rmValue << shift;
		// TODO: Set carry out
	} else {
		not_implemented(__func__, "Data processing instruction");
	}

	switch (opcode) {
		case OPCODE_SUB:
			write_register(rd, read_register(rn) - operand);

			if (s == 1) {
				// TODO: Set flags
				not_implemented(__func__, "Set CPSR");
			}

			break;

		case OPCODE_CMP: {
			int value = read_register(rn) - operand;
			n_flag = value >> 31;
			z_flag = value == 0 ? 1 : 0;

			// TODO: Set c and v flags
			break;
		}

		case OPCODE_MOV:
			write_register(rd, operand);

			if (s == 1) {
				// TODO: Set flags
				not_implemented(__func__, "Set CPSR");
			}

			break;

		default:
			not_implemented(__func__, "Opcode");
			break;
	}
}

// TODO: This needs to fully implement TLBs, etc.
static void execute_load_store_word_or_unsigned_byte(uint32_t instruction) {
	int i = instruction >> 25 & 1;
	int p = instruction >> 24 & 1;
	int u = instruction >> 23 & 1;
	int b = instruction >> 22 & 1;
	int w = instruction >> 21 & 1;
	int l = instruction >> 20 & 1;
	int rn = instruction >> 16 & REGISTER_MASK;
	int rd = instruction >> 12 & REGISTER_MASK;
	int rm = instruction & REGISTER_MASK;
	int offset12 = instruction & 0xfff;
	int shiftImm = instruction >> 7 & 0x1f;
	int shift = instruction >> 5 & 3;

	uint32_t addr = read_register(rn);

	if (p == 1) {				// Immediate offset
		if (u == 1)
			addr += offset12;
		else
			addr -= offset12;
	} else
		not_implemented(__func__, "p = 0");

	if (l == 1) {				// Load
		if (b == 0)				// Word
			write_register(rd, read_word(addr));
		else
			not_implemented(__func__, "Load, b = 1");
	} else { 					// Store
		if (b == 0)				// Word
			write_word(addr, read_register(rd));
		else
			not_implemented(__func__, "Store, b = 1");
	}
}

enum {
	COND_EQ = 0,
	COND_NE = 1,
	COND_AL = 14
};

static void execute_instruction(uint32_t instruction) {
	int cond = instruction >> 28;

	if (cond != COND_AL) {
		switch (cond) {
			case COND_EQ:
				if (z_flag == 0) {
					write_register(pc, read_register(pc) + 4);
					return;
				}
			
			case COND_NE:
				if (z_flag == 1) {
					write_register(pc, read_register(pc) + 4);
					return;
				}
		}
	}

	if ((instruction & DATA_PROCESSING_MASK) == 0)
		execute_data_processing(instruction);
	else if ((instruction & LOAD_STORE_WORD_OR_UNSIGNED_BYTE_MASK) == LOAD_STORE_WORD_OR_UNSIGNED_BYTE)
		execute_load_store_word_or_unsigned_byte(instruction);
	else if ((instruction & BRANCH_MASK) == BRANCH) {
		execute_branch(instruction);
		return;
	} else
		not_implemented(__func__, "Instruction %08x", instruction);

	write_register(pc, read_register(pc) + 4);
}

// Execute the current instruction and increments the PC
void step() {
	uint32_t instruction = fetch_instruction();
	execute_instruction(instruction);
}

void print_cpsr() {
	printf("CPSR: %c%c%c%c\n", n_flag == 0 ? 'n' : 'N', z_flag == 0 ? 'z' : 'Z', c_flag == 0 ? 'c' : 'C', v_flag == 0 ? 'v' : 'V');
}

void print_registers() {
	for (int reg = 0; reg < 16; reg++) {
		printf("%s: ", register_names[reg]);

		if (reg < 10 || reg > 12) {
			printf(" ");
		}

		printf("0x%08x", read_register(reg));

		if (reg == 3 || reg == 7 || reg == 11 || reg == 15) {
			printf("\n");
		} else {
			printf("   ");
		}
	}
}
