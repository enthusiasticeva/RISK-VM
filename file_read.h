#ifndef file_read
#define file_read

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include "memory.h"

int32_t convert_bits_to_num(uint8_t* bits, int length, int twos_comp);
void get_bits(uint8_t* instruction, uint8_t* bits, int start_bit, int end_bit);
int read_in_file(struct instruction* instruction_memory, struct memory_area* data_memory, const char* filename, struct memory_area* instruction_memory_raw);
int instruction_registers_are_valid(struct instruction i);

#endif