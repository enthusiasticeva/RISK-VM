#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include "heap.h"

#define DATA_MEMORY_STARTING_ADDRESS 1024
#define DATA_MEMORY_ENDING_ADDRESS 2047
#define VIRTUAL_ROUTINES_STARTING_ADDRESS 2048
#define VIRTUAL_ROUTINES_ENDING_ADDRESS 2303
#define HEAP_STARTING_ADDRESS 46848


enum TYPE {my_I, my_R, my_S, my_SB, my_U, my_UJ, EMPTY_TYPE, INVALID_TYPE, UNKNOWN_TYPE};
enum INSTRUCTION_NAME {my_add,my_addi,my_sub,my_lui,my_xor,my_xori,my_or,my_ori,my_and,my_andi,my_sll,my_srl,my_sra,my_lb,my_lh,my_lw,my_lbu,my_lhu,my_sb,my_sh,my_sw,my_slt,my_slti,my_sltu,my_sltiu,my_beq,my_bne,my_blt,my_bltu,my_bge,my_bgeu,my_jal,my_jalr,EMPTY_NAME, INVALID_NAME, UNKNOWN_NAME};

struct instruction {
    uint32_t virtual_address;
    enum TYPE type;
    enum INSTRUCTION_NAME name;
    uint8_t opcode;
    uint8_t func3;
    uint8_t func7;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t rd;
    int32_t imm;
};

struct memory_area {
    uint32_t virtual_address;
    uint8_t data;
};

struct machine {
    struct instruction* instruction_memory;
    struct memory_area* instruction_memory_raw;
    struct memory_area* data_memory;
    int32_t* my_registers;
    struct node* heap_head;
    uint32_t program_counter;
};

//helper functions
void init_new_instruction(struct instruction* my_instruction, uint32_t address);
void init_new_memory_area(struct memory_area* my_mem, uint32_t address);
void split_word_to_bytes(uint32_t n, uint8_t* target_array);


void print_registers(int32_t* registers);

//creates an instruction with default values
void init_new_instruction(struct instruction* my_instruction, uint32_t address) {
    struct instruction temp_instruction = {
            .virtual_address = address,
            .type = EMPTY_TYPE,
            .name = EMPTY_NAME,
            .opcode = -1,
            .func3 = -1,
            .func7 = -1,
            .rs1 = -1,
            .rs2 = -1,
            .rd = -1,
            .imm = -1
        };
    *my_instruction = temp_instruction;
}

//creats a new byte of memory with default values
void init_new_memory_area(struct memory_area* my_mem, uint32_t address) {
    struct memory_area temp_mem = {.virtual_address = address};
    *my_mem = temp_mem;
}

//takes a 32 bit number and splits it into it's 4 bytes
void split_word_to_bytes(uint32_t n, uint8_t* target_array) {
    target_array[0] = (n >> 24) & 0x0ff;
    target_array[1] = (n >> 16) & 0x0ff;
    target_array[2] = (n >> 8) & 0x0ff;
    target_array[3] = (n) & 0x0ff;
}

void print_registers(int32_t* registers) {
    int i = 0;
    while (i < 32) {
        printf("R[%d] = 0x%08x;\n",i,registers[i]);
        i ++;
    }
    
}