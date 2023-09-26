#include "heap.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

#define DATA_MEMORY_STARTING_ADDRESS 1024
#define DATA_MEMORY_ENDING_ADDRESS 2047
#define VIRTUAL_ROUTINES_STARTING_ADDRESS 2048
#define VIRTUAL_ROUTINES_ENDING_ADDRESS 2303
#define HEAP_STARTING_ADDRESS 46848


//TYPE: R
void add(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] + machine -> my_registers[rs2];
}

void sub(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] - machine -> my_registers[rs2];
}

void xor_f(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] ^ machine -> my_registers[rs2];
}

void or_f(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] | machine -> my_registers[rs2];
}

void and_f(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] & machine -> my_registers[rs2];
}

void sll(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = (uint32_t)machine -> my_registers[rs1] << machine -> my_registers[rs2];
}

void srl(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = (uint32_t)machine -> my_registers[rs1] >> machine -> my_registers[rs2];
}

void sra(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] >> machine -> my_registers[rs2] | machine -> my_registers[rs1] << (32 - machine -> my_registers[rs2]);
    //based on this stack overflow post: https://stackoverflow.com/a/28303634
}

void slt(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    if (machine -> my_registers[rs1] < machine -> my_registers[rs2]) {
        machine -> my_registers[rd] = 1;
    } else {
        machine -> my_registers[rd] = 0;
    }
    
}

void sltu(struct machine* machine, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t r1 = (uint32_t) machine -> my_registers[rs1];
    uint32_t r2 = (uint32_t) machine -> my_registers[rs2];
    if (r1 < r2) {
        machine -> my_registers[rd] = 1;
    } else {
        machine -> my_registers[rd] = 0;
    }
    
}



//TYPE: I
int lb(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    //calculate mem address
    int32_t mem_address = machine->my_registers[rs1] + imm;

    //check where the memory address points to
    //load from instruction memory
    if (mem_address < DATA_MEMORY_STARTING_ADDRESS && mem_address >= 0) {
        machine->my_registers[rd] = (uint32_t)machine->instruction_memory_raw[mem_address].data;
    }

    //load from data memory
    else if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        machine->my_registers[rd] = machine->data_memory[index].data;
    }
    //VIRTUAL ROUTINES
    //console read char
    else if (mem_address == 0x812) {
        char ch;
        scanf("%c", &ch);
        machine->my_registers[rd] = ch;
    }

    //console read integer
    else if (mem_address == 0x816) {
        int32_t ch;
        scanf("%d", &ch);
        machine->my_registers[rd] = ch;
    }

    //load from heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        machine->my_registers[rd] = current->data[index];
    } 
    //means the memory address was not in any of the legal areas
    else {
        return 1;
    }

    return 0;


    
}

int lh(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    //calculate mem address
    int32_t mem_address = machine->my_registers[rs1] + imm;

    //a half word is 2 bytes
    uint8_t bytes[2];

    //check where the memory address points to
    //load from instruction memory
    if (mem_address < DATA_MEMORY_STARTING_ADDRESS && mem_address >= 0) {
        bytes[0] = machine->instruction_memory_raw[mem_address].data;
        bytes[1] = machine->instruction_memory_raw[mem_address+1].data;
        int32_t result = bytes[0] << 8 | bytes[1];

        machine->my_registers[rd] = result;
    }

    //load from data memory
    else if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        bytes[0] = machine->data_memory[index].data;
        bytes[1] = machine->data_memory[index+1].data;

        // convert the 2 bytes into a number
        int32_t result = bytes[0] << 8 | bytes[1];
        machine->my_registers[rd] = result;
    }

    //console read char
    else if (mem_address == 0x812) {
        char ch;
        scanf("%c", &ch);
        machine->my_registers[rd] = ch;
    }

    //console read integer
    else if (mem_address == 0x816) {
        int32_t ch;
        scanf("%d", &ch);
        machine->my_registers[rd] = ch;
    }

    //load from heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        bytes[0] = machine->data_memory[index].data;

        //check if the two bytes are in the same memory bank
        if (index + 1 < 64) {
            bytes[1] = machine->data_memory[index+1].data;
        } else {
            current = current->next;
            bytes[1] = machine->data_memory[0].data;
        }
        
        // convert the 2 bytes into a number
        int32_t result = bytes[0] << 8 | bytes[1];
        machine->my_registers[rd] = result;

    }
    //means the memory address was not in any of the legal areas
    else {
        return 1;
    }
    return 0;

}

int lw(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    int32_t mem_address = machine->my_registers[rs1] + imm;

    uint8_t bytes[4];

    if (mem_address < DATA_MEMORY_STARTING_ADDRESS && mem_address >= 0) {
        bytes[0] = machine->instruction_memory_raw[mem_address].data;
        bytes[1] = machine->instruction_memory_raw[mem_address+1].data;
        bytes[2] = machine->instruction_memory_raw[mem_address+2].data;
        bytes[3] = machine->instruction_memory_raw[mem_address+3].data;
        int32_t result = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];

        machine->my_registers[rd] = result;
    }

    //load from data memory
    else if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        bytes[0] = machine->data_memory[index].data;
        bytes[1] = machine->data_memory[index+1].data;
        bytes[2] = machine->data_memory[index+2].data;
        bytes[3] = machine->data_memory[index+3].data;

        // convert the 4 bytes into a number
        int32_t result = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
        
        machine->my_registers[rd] = result;

    }

    //console read char
    else if (mem_address == 0x812) {
        char ch;
        scanf("%c", &ch);
        machine->my_registers[rd] = ch;
    }

    //console read integer
    else if (mem_address == 0x816) {
        int32_t ch;
        scanf("%d", &ch);
        machine->my_registers[rd] = ch;
    }

    //load from heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        // if the initial index is less than 60, all 4 bytes are in the same heap, 
        // so we can retrive them without moving banks
        if (index < 60) {
            int i = 0;
            while (i < 4) {
                bytes[i] = current->data[index+i];
                i ++;
            }
        //if he initial index is not less than 60, then we need to move to the 
        // next bank when the index becomes 64+
        } else {
            int i = 0;
            while (i < 4) {
                if (index > 63) {
                    current = current->next;
                    index = 0;
                }
                bytes[i] = current->data[index+i];
                index ++;
                i ++;
            }
        }
        
        // convert the 4 bytes into a number
        int32_t result = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
        machine->my_registers[rd] = result;
    }
    //means the memory address was not in any of the legal areas
    else {
        return 1;
    }
    return 0;
}

int lbu(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    //calculate mem address
    int32_t mem_address = machine->my_registers[rs1] + imm;

    //check where the memory address points to
    //load from instruction memory
    if (mem_address < DATA_MEMORY_STARTING_ADDRESS && mem_address >= 0) {
        machine->my_registers[rd] = (uint32_t)machine->instruction_memory_raw[mem_address].data;
    }

    //load from data memory
    else if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        machine->my_registers[rd] = (uint32_t)machine->data_memory[index].data;
    }

    //console read char
    else if (mem_address == 0x812) {
        char ch;
        scanf("%c", &ch);
        machine->my_registers[rd] = ch;
    }

    //console read integer
    else if (mem_address == 0x816) {
        int32_t ch;
        scanf("%d", &ch);
        machine->my_registers[rd] = ch;
    }

    //load from heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        machine->my_registers[rd] = (uint32_t)current->data[index];
    }
    //means the memory address was not in any of the legal areas
    else {
        return 1;
    }
    return 0;
}

int lhu(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    //calculate mem address
    int32_t mem_address = machine->my_registers[rs1] + imm;

    uint8_t bytes[2];

    //check where the memory address points to
    //load from instruction memory
    if (mem_address < DATA_MEMORY_STARTING_ADDRESS && mem_address >= 0) {
        bytes[0] = machine->instruction_memory_raw[mem_address].data;
        bytes[1] = machine->instruction_memory_raw[mem_address+1].data;
        uint32_t result = bytes[0] << 8 | bytes[1];

        machine->my_registers[rd] = result;
    }

    //load from data memory
    else if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        bytes[0] = machine->data_memory[index].data;
        bytes[1] = machine->data_memory[index+1].data;

        // convert the 2 bytes into a number
        uint32_t result = bytes[0] << 8 | bytes[1];
        machine->my_registers[rd] = result;
    }

    //console read char
    else if (mem_address == 0x812) {
        char ch;
        scanf("%c", &ch);
        machine->my_registers[rd] = ch;
    }

    //console read integer
    else if (mem_address == 0x816) {
        int32_t ch;
        scanf("%d", &ch);
        machine->my_registers[rd] = ch;
    }

    //load from heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        bytes[0] = machine->data_memory[index].data;

        //check if the two bytes are in the same memory bank, if not, move between banks
        if (index + 1 < 64) {
            bytes[1] = machine->data_memory[index+1].data;
        } else {
            current = current->next;
            bytes[1] = machine->data_memory[0].data;
        }
        
        // convert the 2 bytes into a number
        uint32_t result = bytes[0] << 8 | bytes[1];
        
        machine->my_registers[rd] = result;
    }

    //means the memory address was not in any of the legal areas
    else {
        return 1;
    }
    return 0;

}

void addi(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] + imm;
}

void xori(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] ^ imm;
}

void ori(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] | imm;
}

void andi(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    machine -> my_registers[rd] = machine -> my_registers[rs1] & imm;
}

void slti(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    if (machine -> my_registers[rs1] < imm) {
        machine -> my_registers[rd] = 1;
    } else {
        machine -> my_registers[rd] = 0;
    }
}

void sltiu(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    uint32_t r1 = (uint32_t) machine -> my_registers[rs1];
    uint32_t r2 = (uint32_t) imm;
    if (r1 < r2) {
        machine -> my_registers[rd] = 1;
    } else {
        machine -> my_registers[rd] = 0;
    }
}

//save the incremented PC to rd, and add imm to the program counter
void jalr(struct machine* machine, uint8_t rs1, int32_t imm, uint8_t rd) {
    machine -> my_registers[rd] = machine->program_counter+4;
    machine->program_counter = machine->my_registers[rs1] + imm;
}



//TYPE: SB
void beq(struct machine* machine,uint8_t rs1, uint8_t rs2, int32_t imm) {
    if (machine -> my_registers[rs1] == machine -> my_registers[rs2]) {
        machine -> program_counter += imm << 1;
    } else {
        machine -> program_counter += 4;
    }
}

void bne(struct machine* machine,uint8_t rs1, uint8_t rs2, int32_t imm) {
    if (machine -> my_registers[rs1] != machine -> my_registers[rs2]) {
        machine -> program_counter += imm << 1;
    } else {
        machine -> program_counter += 4;
    }
}

void blt(struct machine* machine,uint8_t rs1, uint8_t rs2, int32_t imm) {
    if ((int32_t)machine -> my_registers[rs1] < (int32_t)machine -> my_registers[rs2]) {
        machine -> program_counter += imm << 1;
    } else {
        machine -> program_counter += 4;
    }
}

void bltu(struct machine* machine,uint8_t rs1, uint8_t rs2, int32_t imm) {
    if ((uint32_t) machine -> my_registers[rs1] < (uint32_t) machine -> my_registers[rs2]) {
        machine -> program_counter += imm << 1;
    } else {
        machine -> program_counter += 4;
    }
}

void bge(struct machine* machine,uint8_t rs1, uint8_t rs2, int32_t imm) {
    if ((int32_t)machine -> my_registers[rs1] >=(int32_t)machine -> my_registers[rs2]) {
        machine -> program_counter += imm << 1;
    } else {
        machine -> program_counter += 4;
    }
}

void bgeu(struct machine* machine,uint8_t rs1, uint8_t rs2, int32_t imm) {
    if ((uint32_t) machine -> my_registers[rs1] >= (uint32_t) machine -> my_registers[rs2]) {
        machine -> program_counter += imm << 1;
    } else {
        machine -> program_counter += 4;
    }
}

//TYPE U
void lui(struct machine* machine, int32_t imm, uint8_t rd) {
    machine -> my_registers[rd] = imm;
}

//TYPE: UJ
void jal(struct machine* machine, uint8_t rd, int32_t imm) {
    machine -> my_registers[rd] = machine->program_counter+4;
    machine->program_counter = machine->program_counter + (imm << 1);
}



//TYPE: S
int sw(struct machine* machine, uint8_t rs1, uint8_t rs2, int32_t imm) {

   //calculate mem address
    int32_t mem_address = machine->my_registers[rs1] + imm;

    uint8_t bytes[4];
    split_word_to_bytes(machine->my_registers[rs2], bytes);

    //check where the memory address points to
    //data memory
    if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        //add the 4 bytes to memory
        int i = 0;
        while (i < 4) {
            machine->data_memory[index+i].data = bytes[i];
            i ++;
        }

    }

    //virtual routines
    //console write character
    else if (mem_address == 0x800) {
       
        printf("%c", machine->my_registers[rs2]);
    }

    //console write signed integer
    else if (mem_address == 0x804) {
        printf("%d", machine->my_registers[rs2]);
    }

    //console write unsigned integer
    else if (mem_address == 0x808) {
        printf("%x", (uint32_t)machine->my_registers[rs2]);
    }

    //dump PC
    else if (mem_address == 0x820) {
        printf("PC = 0x%08x;\n", machine->program_counter);
    }

    //dump register banks
    else if (mem_address == 0x824) {
        printf("PC = 0x%08x;\n", machine->program_counter);
        print_registers(machine->my_registers);
    }



    //heap - malloc, this should malloc a chunk of the size rs2, and put address in r[28]
    else if (mem_address == 0x830) {
        //malloc the required space
        uint32_t virtual_address = my_malloc(machine->heap_head, machine->my_registers[rs2]);
        //my_malloc will return 0 if it was unsuccessful, so R[28] is set to 0 if there is a failure
        machine->my_registers[28] = virtual_address;
        
    }

    //heap - free
    else if (mem_address == 0x834) {
        uint32_t virtual_address = machine->my_registers[rs2];
        int fail = my_free(machine->heap_head, virtual_address);

        //if freeing doesnt work, an illegal operation is raised, through returning 1
        if (fail == true) {
            return 1;
        }
    }

    //store in heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        //if the index is large enough, we will need to flow into the next bank.
        if (index < 59) {
            int i = 0;
            while (i < 4) {
                current->data[index+i] = bytes[i];
                i ++;
            }
        } else {
            int i = 0;
            while (i < 4) {
                if (index > 63) {
                    current = current->next;
                    index = 0;
                }
                current->data[index] = bytes[i];
                index ++;
                i ++;
            }
        }
    }
    //means illegal mem address
    else {
        
        return 1;
    }
    return 0;
    
}

int sh(struct machine* machine, uint8_t rs1, uint8_t rs2, int32_t imm) { 
    //check if the mem address in the heap or stack...
    int32_t mem_address = machine->my_registers[rs1] + imm;

    //we will only use the last 2 bytes of this
    uint8_t bytes[4];
    split_word_to_bytes(machine->my_registers[rs2], bytes);

    //data memory
    if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        //will access 2 and 4 TODO try the other 2 bytes
        int i = 2;
        while (i < 4) {
            machine->data_memory[index+i].data = bytes[i];
            i ++;
        }

    }

    //virtual routines
    //console write character
    else if (mem_address == 0x800) {
        printf("%c", machine->my_registers[rs2]);
    }

    //console write signed integer
    else if (mem_address == 0x804) {
        printf("%d", machine->my_registers[rs2]);
    }

    //console write unsigned integer
    else if (mem_address == 0x808) {
        printf("%x", (uint32_t)machine->my_registers[rs2]);
    }

    //dump PC
    else if (mem_address == 0x820) {
        printf("PC = 0x%08x;\n", machine->program_counter);
    }

    //dump register banks
    else if (mem_address == 0x824) {
        printf("PC = 0x%08x;\n", machine->program_counter);
        print_registers(machine->my_registers);
    }



    //heap - malloc, this should malloc a chunk of the size rs2, and put address in r[28]
    else if (mem_address == 0x830) {
        //malloc the required space
        uint32_t virtual_address = my_malloc(machine->heap_head, machine->my_registers[rs2]);

        machine->my_registers[28] = virtual_address;//this might need to be 28, unclear to me whether registers start form 0 TODO
        
    }

    //heap - free
    else if (mem_address == 0x834) {
        uint32_t virtual_address = machine->my_registers[rs2];
        int fail = my_free(machine->heap_head, virtual_address);
        
        //if freeing doesnt work, an illegal operation is raised, through returning 1
        if (fail == true) {
            return 1;
        }
    }

    //store in heap
    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        //go to the bank where the address is contained
        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        //check this memory has been allocated
        if (current->unused == true) {
            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the right bank, then to the right byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        //if the index is large enough, we will need to flow into the next bank.
        if (index < 59) {
            int i = 2;
            while (i < 4) {
                current->data[index+i] = bytes[i];
                i ++;
            }
        } else {
            int i = 2;
            while (i < 4) {
                if (index > 63) {
                    current = current->next;
                    index = 0;
                }
                current->data[index] = bytes[i];
                index ++;
                i ++;
            }
        }
    }
    //means illegal mem address
    else {
        
        return 1;
    }
    return 0;
}

int sb(struct machine* machine, uint8_t rs1, uint8_t rs2, int32_t imm) {
    
    //check if the mem address in the heap or stack...
    int32_t mem_address = machine->my_registers[rs1] + imm;

    //data memory
    if (mem_address >= DATA_MEMORY_STARTING_ADDRESS && mem_address <= DATA_MEMORY_ENDING_ADDRESS) {
        int index = mem_address - 1024;
        machine->data_memory[index].data = imm;
        
    }

    //virtual routines
    //console write character
    else if (mem_address == 0x800) {
        
        printf("%c", machine->my_registers[rs2]);
    }

    //console write signed integer
    else if (mem_address == 0x804) {
        printf("%d", machine->my_registers[rs2]);
    }

    //console write unsigned integer
    else if (mem_address == 0x808) {
        printf("%x", (uint32_t)machine->my_registers[rs2]);
    }

    //dump PC
    else if (mem_address == 0x820) {
        printf("PC = 0x%08x;\n", machine->program_counter);
    }

    //dump register banks
    else if (mem_address == 0x824) {
        printf("PC = 0x%08x;\n", machine->program_counter);
        print_registers(machine->my_registers);
    }

    //heap - malloc, this should malloc a chunk of the size rs2, and put address in r[28]
    else if (mem_address == 0x830) {
        //malloc the required space
        uint32_t virtual_address = my_malloc(machine->heap_head, machine->my_registers[rs2]);

        machine->my_registers[28] = virtual_address;//this might need to be 28, unclear to me whether registers start form 0 TODO
        
    }

    //heap - free
    else if (mem_address == 0x834) {
        uint32_t virtual_address = machine->my_registers[rs2];
        int fail = my_free(machine->heap_head, virtual_address);

        //if freeing doesnt work, an illegal operation is raised, through returning 1
        if (fail == true) {
            return 1;
        }
    }

    else if (mem_address >= HEAP_STARTING_ADDRESS && mem_address < HEAP_STARTING_ADDRESS+8192) {
        struct node* current = machine->heap_head;

        while (current->virtual_address > mem_address) {
            current = current->next;
        }

        if (current->unused == true) {            
            return 1;
        }

        //as the heap is made up of 64 byte banks, we need to go to the correct bank, then to the correct byte.
        uint32_t index = (mem_address - HEAP_STARTING_ADDRESS)%64;

        //if the index is large enough, we will need to flow into the next bank.
        current->data[index] = imm;
    }
    //means illegal mem address
    else {
        
        return 1;
    }
    return 0;
    
}