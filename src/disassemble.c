///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
///////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "disassemble.h"
#include "error.h"
#include "memory.h"

// Buffer for disassembly
static char buffer[128];
static char *buf_ptr;

static char *conditions[] = { "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "  ", "  " };

static char *condition_string(uint32_t instruction) {
    return conditions[(instruction >> CONDITION_SHIFT) & CONDITION_MASK];
}

static void disassemble_branch(uint32_t addr, uint32_t instruction) {
	int l = instruction >> 24 & 1;
	int signed_immed24 = instruction & 0x00FFFFFF;

	uint32_t target_address = ((signed_immed24 << 8) >> 6) + program_counter();
    sprintf(buf_ptr, "%s%s%s%x", l == 0 ? "b" : "bl", condition_string(instruction), l == 0 ? "     " : "    ", target_address);
}

static char *opcodes[] = { "and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc", "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn" };

static void disassemble_data_processing(uint32_t instruction) {
    int i = instruction >> 25 & 1;
	int opcode = instruction >> 21 & OPCODE_MASK;
	int s = instruction >> 20 & 1;
	int rn = instruction >> 16 & REGISTER_MASK;
	int rd = instruction >> 12 & REGISTER_MASK;

	if (opcode == OPCODE_MOV) {
		if (i == 0 && (instruction & DATA_PROCESSING_LSL_MASK) == 0)
			strcpy(buf_ptr, "lsl");
		else
			strcpy(buf_ptr, "mov");
    } else
		strcpy(buf_ptr, opcodes[opcode]);

    buf_ptr += 3;
    buf_ptr += sprintf(buf_ptr, "%s", condition_string(instruction));
    
    if (s == 1 && opcode != OPCODE_CMP) {
        if (*(buf_ptr - 1) == ' ') {
            *(buf_ptr - 2) = 's';
            *buf_ptr++ = ' ';
        }
    } else
        *buf_ptr++ = ' ';

    buf_ptr += sprintf(buf_ptr, "  %s, ", register_names[rd]);

	uint32_t operand;

	if (i == 1) {
		int rotate = (instruction >> 8 & ROTATE_MASK) << 1;
		int immediate = instruction & IMMEDIATE_MASK;
		operand = (immediate >> rotate) | (immediate << (32 - rotate));

        if (opcode == OPCODE_MOV)
            buf_ptr += sprintf(buf_ptr, "#%d", operand);
        else
            buf_ptr += sprintf(buf_ptr ,"%s, #%d", register_names[rn], operand);        
	} else if ((instruction & DATA_PROCESSING_LSL_MASK) == 0) {
		int shift = instruction >> 7 & SHIFT_MASK;
		int rm = instruction & REGISTER_MASK;

        if (opcode == OPCODE_MOV)
            buf_ptr += sprintf(buf_ptr, "%s, #%d", register_names[rm], shift);
        else
            buf_ptr += sprintf(buf_ptr ,"%s, lsl #%d", register_names[rm], shift);
	} else {
		not_implemented(__func__, "Data processing instruction");
	}
}

static void disassemble_load_store_word_or_unsigned_byte(uint32_t addr, uint32_t instruction) {
	int i = (instruction >> 25) & 1;
	int p = (instruction >> 24) & 1;
	int u = (instruction >> 23) & 1;
	int b = (instruction >> 22) & 1;
	int w = (instruction >> 21) & 1;
	int l = (instruction >> 20) & 1;
	int rn = (instruction >> 16) & REGISTER_MASK;
	int rd = (instruction >> 12) & REGISTER_MASK;
	int rm = instruction & REGISTER_MASK;
	int offset12 = instruction & 0xfff;
	int shift_imm = (instruction >> 7) & 0x1f;
	int shift = (instruction >> 5) & 3;

    if (l == 1)
        buf_ptr += sprintf(buf_ptr, "ldr%s", condition_string(instruction));
    else
        buf_ptr += sprintf(buf_ptr, "str%s", condition_string(instruction));

    int num_spaces = 0;

    if (b == 1)
        *buf_ptr++ = 'b';
    else
        num_spaces++;
    
    if (p == 0 && w == 1)
        *buf_ptr++ = 't';
    else
        num_spaces++;

    if (num_spaces >= 1)
        *buf_ptr++ = ' ';

    if (num_spaces >= 2)
        *buf_ptr++ = ' ';
    
    buf_ptr += sprintf(buf_ptr, " %s, ", register_names[rd]);

    if (i == 0) {
        buf_ptr += sprintf(buf_ptr, "[%s, #", register_names[rn]);

        if (u == 0) {
            buf_ptr += sprintf(buf_ptr, "-%d]", offset12);
            addr -= offset12;
        } else {
            buf_ptr += sprintf(buf_ptr, "%d]", offset12);
            addr += offset12;
        }

        if (rn == pc)
            buf_ptr += sprintf(buf_ptr, "   ; %x", addr + 8);           // 8 byte pipeline
        else if (offset12 > 15)
            buf_ptr += sprintf(buf_ptr, "   ; 0x%x", offset12); 
    } else {
        not_implemented(__func__, "i == 1");
        assert(0);
    }
}

char *disassemble(uint32_t addr) {
    uint32_t instruction = read_word(addr);
    sprintf(buffer, "%8x:  %08x  ", addr, instruction);
    buf_ptr = buffer + 21;  // Length of above string

    if ((instruction & DATA_PROCESSING_MASK) == 0)
        disassemble_data_processing(instruction);
    else if ((instruction & LOAD_STORE_WORD_OR_UNSIGNED_BYTE_MASK) == LOAD_STORE_WORD_OR_UNSIGNED_BYTE)
        disassemble_load_store_word_or_unsigned_byte(addr, instruction);
    else if ((instruction & BRANCH_MASK) == BRANCH)
        disassemble_branch(addr, instruction);
    else
        sprintf(buf_ptr, ".word   0x%08x", instruction);

    return buffer;
}
