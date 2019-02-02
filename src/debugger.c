///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
///////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "debugger.h"
#include "disassemble.h"

static void display_prompt() {
    printf("> ");
}

static int step_count = 0;         // -1 means run forever, 0 means break into debugger, > 0 means execute instruction (this will decrement each instruction)

enum { BUFFER_LENGTH = 80 };

static char input_buffer[BUFFER_LENGTH];
static char previous_input[BUFFER_LENGTH] = "s\n";       // Previous action is to single step

// Setting to true will quit the debugger
static bool quit = false;

static void debug () {
    printf("%s\n", disassemble(program_counter() - 8));
    bool done = false;

    while (!quit && !done) {
        display_prompt();
        fgets(input_buffer, BUFFER_LENGTH, stdin);

        char *input = input_buffer;

        if (*input == '\n')
            input = previous_input;

        switch (*input) {
            case 'c':
                print_cpsr();
                break;

            case 'g':
                step_count = -1;
                
                if (previous_input != input)
                    strcpy(previous_input, input);

                done = true;
                break;

            case 'l':
                printf("%s\n", disassemble(program_counter() - 8));
                break;

            case 'q':
                quit = true;
                break;

            case 'r':
                print_registers();
                break;

            case 's':
                step_count = 1;

                if (previous_input != input)
                    strcpy(previous_input, input);

                done = true;
                break;

            default:
                printf("Unknown command: %c\n", *input);
                break;
        }
    }
}

void run() {
    while (true) {
        if (step_count == 0) {
            debug();

            if (quit)
                exit(0);
        } else {
            step();

            if (step_count > 0)
                step_count--;
        }
    }
}