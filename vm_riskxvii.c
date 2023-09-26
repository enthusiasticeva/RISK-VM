#include "heap.h"
#include "memory.h"

#include "instruction_functions.h"
#include "file_read.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
int main(int argc, char const *argv[]) {

    struct machine my_machine;

    //MAKE MEMORY
    my_machine.program_counter = 0;

    struct instruction instruction_memory[256];
    my_machine.instruction_memory = instruction_memory;
    
    struct memory_area data_memory[1024];
    my_machine.data_memory = data_memory;

    struct memory_area instruction_memory_raw[1024];
    my_machine.instruction_memory_raw = instruction_memory_raw;

    int32_t my_registers[32];
    my_registers[0] = 0;
    my_machine.my_registers = my_registers;


    struct node* head = create_heap();  
    my_machine.heap_head = head; 

    //set up the instruction memory with the appropriate addresses from 0-1023 (going up by 4)
    int i = 0;
    while (i < 256) {
        init_new_instruction(&instruction_memory[i], i*4);
        i ++;
    }

    //set up the data memory with the appropriate addresses from 1024-2048 (going up by 1)
    i = 0;
    while (i<1024){
        init_new_memory_area(&data_memory[i], 1024 + (i));
        i++;
    }

    //set up the raw instruction memory with the appropriate addresses from 1024-2048 (going up by 1)
    i = 0;
    while (i<1024){
        init_new_memory_area(&instruction_memory_raw[i], i);
        i++;
    }

    //instruction memory (both in its raw form and in its form as instruction structs), data memory are populated by this function.
    read_in_file(instruction_memory, data_memory, argv[1], instruction_memory_raw);

    
    // Loop through instructions
    while (true) {
        //get instruction
        struct instruction current_instruction = instruction_memory[my_machine.program_counter/4];

        //all memory operations return 1 if the memory address given was out of range. This variable will keep track of this
        int memory_fail = false;
        //check for HALT
        if (current_instruction.type == my_S && my_machine.my_registers[current_instruction.rs1] +current_instruction.imm == 0x80c) {
            printf("CPU Halt Requested\n");
            break;
        }

        //if there is no halt, we should stop at the end of instruction memory
        if (my_machine.program_counter >= 1024) {
            break;
        }

        if (current_instruction.name == EMPTY_NAME) {
            my_machine.program_counter += 4;
            continue;
        }


        //check if the file read found a unknown/illegal instruction
        //As the code early exits if this is triggered, the heap needs to be freed.
        if (current_instruction.name == UNKNOWN_NAME) {
            printf("Instruction Not Implemented: 0x%02x%02x%02x%02x\n", 
                my_machine.instruction_memory_raw[my_machine.program_counter+3].data, 
                my_machine.instruction_memory_raw[my_machine.program_counter+2].data, 
                my_machine.instruction_memory_raw[my_machine.program_counter+1].data, 
                my_machine.instruction_memory_raw[my_machine.program_counter].data);

            printf("PC = 0x%08x;\n", my_machine.program_counter);
            print_registers(my_machine.my_registers);

            free_linked_list(my_machine.heap_head);
            return 1;
        } 
        else if (current_instruction.name == INVALID_NAME) {
            printf("Illegal Operation: 0x%02x%02x%02x%02x\n", 
                my_machine.instruction_memory_raw[my_machine.program_counter+3].data, 
                my_machine.instruction_memory_raw[my_machine.program_counter+2].data, 
                my_machine.instruction_memory_raw[my_machine.program_counter+1].data, 
                my_machine.instruction_memory_raw[my_machine.program_counter].data);

            printf("PC = 0x%08x;\n", my_machine.program_counter);
            print_registers(my_machine.my_registers);

            free_linked_list(my_machine.heap_head);
            return 1;
        }

        //check for jump or branch instructions, in which case the PC doesnt need to be incremented
        else if (current_instruction.name == my_jal) {
            jal(&my_machine, current_instruction.rd, current_instruction.imm);
            //set the zero register back to 0
            my_machine.my_registers[0] = 0;
        } 
        
        else if (current_instruction.name == my_jalr) {
            jalr(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
            //set the zero register back to 0
            my_machine.my_registers[0] = 0;
        }

        else if (current_instruction.type == my_SB) {
            switch (current_instruction.name) {
                case my_beq:
                    beq(&my_machine, current_instruction.rs1,current_instruction.rs2,current_instruction.imm);
                    break;

                case my_bne:
                    bne(&my_machine, current_instruction.rs1,current_instruction.rs2,current_instruction.imm);
                    break;

                case my_blt:
                    blt(&my_machine, current_instruction.rs1,current_instruction.rs2,current_instruction.imm);
                    break;

                case my_bltu:
                    bltu(&my_machine, current_instruction.rs1,current_instruction.rs2,current_instruction.imm);
                    break;

                case my_bge:
                    bge(&my_machine, current_instruction.rs1,current_instruction.rs2,current_instruction.imm);
                    break;

                case my_bgeu:
                    bgeu(&my_machine, current_instruction.rs1,current_instruction.rs2,current_instruction.imm);
                    break;

                default:
                    break;
            }
        }

        //All the other instructions dont affect the PC, so we can just increment by 4
        else {

            switch (current_instruction.name) {
                //type: I
                case my_lb:
                    memory_fail = lb(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_lh:
                    memory_fail = lh(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_lw:
                    memory_fail = lw(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_lbu:
                    memory_fail = lbu(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_lhu:
                    memory_fail = lhu(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_addi:
                    addi(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_xori:
                    xori(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_ori:
                    ori(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_andi:
                    andi(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_slti:
                    slti(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                case my_sltiu:
                    sltiu(&my_machine, current_instruction.rs1, current_instruction.imm, current_instruction.rd);
                    break;
                
                //type : R
                case my_add:
                    add(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_sub:
                    sub(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_xor:
                    xor_f(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_or:
                    or_f(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_and:
                    and_f(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_sll:
                    sll(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_srl:
                    srl(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_sra:
                    sra(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_slt:
                    slt(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                case my_sltu:
                    sltu(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.rd);
                    break;
                
                //type:S
                case my_sb:
                    memory_fail = sb(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.imm);
                    break;
                
                
                case my_sh:
                    memory_fail = sh(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.imm);
                    break;
                
                case my_sw:
                    memory_fail = sw(&my_machine, current_instruction.rs1, current_instruction.rs2, current_instruction.imm);
                    break;
                
                //type: U
                case my_lui:
                    lui(&my_machine, current_instruction.imm, current_instruction.rd);
                    break;
                
                default:
                    break;
            }

            
            
            //set the zero register back to 0
            my_machine.my_registers[0] = 0;

            //if the operation was a memory one and na illegal address was give, then memory fail would be 1.
            //As the code early exits if this is triggered, the heap needs to be freed.
            if (memory_fail == true) {
                printf("Illegal Operation: 0x%02x%02x%02x%02x\n", 
                    my_machine.instruction_memory_raw[my_machine.program_counter+3].data,
                    my_machine.instruction_memory_raw[my_machine.program_counter+2].data, 
                    my_machine.instruction_memory_raw[my_machine.program_counter+1].data, 
                    my_machine.instruction_memory_raw[my_machine.program_counter].data);

                printf("PC = 0x%08x;\n", my_machine.program_counter);
                print_registers(my_machine.my_registers);

                free_linked_list(my_machine.heap_head);
                return 1;
            }

            //increment the program counter
            my_machine.program_counter += 4;
        }

        
    }


    //FREE MEMORY
    free_linked_list(my_machine.heap_head);
}