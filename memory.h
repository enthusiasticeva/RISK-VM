#ifndef my_memory
#define my_memory

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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

//print functions
void print_registers(int32_t* registers);



#endif