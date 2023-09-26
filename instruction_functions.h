#ifndef instruction_functions
#define instruction_functions

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

#include "heap.h"
#include "memory.h"


#define DATA_MEMORY_STARTING_ADDRESS 1024
#define DATA_MEMORY_ENDING_ADDRESS 2047
#define VIRTUAL_ROUTINES_STARTING_ADDRESS 2048
#define VIRTUAL_ROUTINES_ENDING_ADDRESS 2303
#define HEAP_STARTING_ADDRESS 46848

//INSTRUCTION FUNCTIONS

//TYPE: R
void add(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void sub(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void xor_f(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void or_f(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void and_f(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void sll(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void srl(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void sra(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void slt(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

void sltu(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd);

//TYPE: I
int lb(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

int lh(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

int lw(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

int lbu(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

int lhu(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void addi(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void xori(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void ori(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void andi(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void slti(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void sltiu(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);

void jalr(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd);



//TYPE: SB
void beq(struct machine* machine,uint8_t rs1, uint8_t rs2, uint32_t imm) ;

void bne(struct machine* machine,uint8_t rs1, uint8_t rs2, uint32_t imm);

void blt(struct machine* machine,uint8_t rs1, uint8_t rs2, uint32_t imm);

void bltu(struct machine* machine,uint8_t rs1, uint8_t rs2, uint32_t imm);

void bge(struct machine* machine,uint8_t rs1, uint8_t rs2, uint32_t imm);

void bgeu(struct machine* machine,uint8_t rs1, uint8_t rs2, uint32_t imm);

//TYPE U
void lui(struct machine* machine, int32_t imm, uint8_t rd);

//TYPE: UJ
void jal(struct machine* machine, uint8_t rd, uint32_t imm);



//TYPE: S
int sw(struct machine* machine, uint8_t rs1, uint8_t rs2, uint32_t imm);

int sb(struct machine* machine, uint8_t rs1, uint8_t rs2, uint32_t imm);

int sh(struct machine* machine, uint8_t rs1, uint8_t rs2, uint32_t imm);

#endif