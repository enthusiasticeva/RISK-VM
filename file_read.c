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

//takes an array of 1s and 0s and converts it to an decimal number
int32_t convert_bits_to_num(uint8_t* bits, int length, int twos_comp) {
    uint32_t result = 0;
    uint32_t i = 0;

    
    while (i < length) {
    
        uint32_t temp = bits[i]* (1<<(length-1-i));
        result += temp; //(1<<i) = 2^i
        i++;
    }
    
    if (twos_comp == 1 && bits[0] == 1) {
        result = result - (1<<length); 
        //this technique for converting from 2's complement is taken from https://www.exploringbinary.com/twos-complement-converter/ TODO proper reference
    }
    

    return result;
}


//returns an array of bits from start_bit to end_bit, inclusive
void get_bits(uint8_t* instruction, uint8_t* bits, int start_bit, int end_bit) {
    int length = end_bit-start_bit+1;

    int d = 0;
    while (d < length) {
        bits[d] = instruction[31-end_bit+d];
        d++;
    }
}

int read_in_file(struct instruction* instruction_memory, struct memory_area* data_memory, const char* filename, struct memory_area* instruction_memory_raw) {
    FILE* file = fopen(filename, "rb");

    int instruction_index = 0;
    int memory_index = 0;
    int bytes_read = 0;

    char* line_buffer = malloc(4*sizeof(char));

    while (fread(line_buffer, 4*sizeof(char),1, file)) {
        bytes_read += 4;
        if (bytes_read == 2048) {
            break;
        }
        if (instruction_index < 256) {

            //get the instrction as an array of 1s and 0s
            uint8_t bits[32];
            int len = 0;

            int byte = 3;

            while (byte >= 0) {
                int i = 0;
                while (i < 8) {
                    int bit = line_buffer[byte] >> (7-i) & 1;
                    bits[len] = bit;

                    len ++;
                    i++;
                }
                byte--;
            } 


            //GET OPCODE
            uint8_t opcode_bits[7];

            int opcode_starting_bit = 0;
            int opcode_ending_bit = 6;
            get_bits(bits, opcode_bits, opcode_starting_bit, opcode_ending_bit);

            

            uint8_t opcode = convert_bits_to_num(opcode_bits, 7, 0);
            instruction_memory[instruction_index].opcode = opcode;

            //CHECK OPCODE
            //-----------------------------
            //TYPE: I
            if (opcode == 19 || opcode == 3 || opcode == 103 ) {
                instruction_memory[instruction_index].type = my_I;

                //GET RD - bits 7-11
                uint8_t rd_bits[5];
                get_bits(bits, rd_bits, 7, 11);
                instruction_memory[instruction_index].rd = convert_bits_to_num(rd_bits, 5, 0);

                //FUNC3 - bits 12-14
                uint8_t func3_bits[3];
                get_bits(bits, func3_bits, 12, 14);
                instruction_memory[instruction_index].func3 = convert_bits_to_num(func3_bits, 3, 0);

                //RS1 - bits 15-19
                uint8_t rs1_bits[5];
                get_bits(bits, rs1_bits, 15, 19);
                instruction_memory[instruction_index].rs1 = convert_bits_to_num(rs1_bits, 5, 0);

                //IMM - bits 20-31
                uint8_t imm_bits[12];
                get_bits(bits, imm_bits, 20, 31);
                instruction_memory[instruction_index].imm = convert_bits_to_num(imm_bits, 12, 1);

                if (opcode == 103) {
                    instruction_memory[instruction_index].name=my_jalr;
                }

                if (opcode == 3) {
                    switch (instruction_memory[instruction_index].func3) {
                        case 0:
                            instruction_memory[instruction_index].name=my_lb;
                            break;

                        case 1:
                            instruction_memory[instruction_index].name=my_lh;
                            break;
                        
                        case 2:
                            instruction_memory[instruction_index].name=my_lw;
                            break;
                        
                        case 4:
                            instruction_memory[instruction_index].name=my_lbu;
                            break;
                        
                        case 5:
                            instruction_memory[instruction_index].name=my_lhu;
                            break;
                        default:
                            instruction_memory[instruction_index].type = UNKNOWN_TYPE;
                            instruction_memory[instruction_index].name = UNKNOWN_NAME;
                            break;
                        
                        
                    }
                }

                if (opcode == 19) {
                    switch (instruction_memory[instruction_index].func3) {
                        case 0:
                            instruction_memory[instruction_index].name=my_addi;
                            break;
                        
                        case 4:
                            instruction_memory[instruction_index].name=my_xori;
                            break;
                        
                        case 6:
                            instruction_memory[instruction_index].name=my_ori;
                            break;
                        
                        case 7:
                            instruction_memory[instruction_index].name=my_andi;
                            break;
                        
                        case 2:
                            instruction_memory[instruction_index].name=my_slti;
                            break;
                        
                        case 3:
                            instruction_memory[instruction_index].name=my_sltiu;
                            break;

                        default:
                            instruction_memory[instruction_index].type = UNKNOWN_TYPE;
                            instruction_memory[instruction_index].name = UNKNOWN_NAME;
                            break;
                        
                        
                    }
                }
            }

            //-----------------------------
            //TYPE: R
            else if (opcode == 51) {
                instruction_memory[instruction_index].type = my_R;

                //RD - bits 7-11
                uint8_t rd_bits[5];
                get_bits(bits, rd_bits, 7, 11);
                instruction_memory[instruction_index].rd = convert_bits_to_num(rd_bits, 5, 0);

                //FUNC3 - bits 12-14
                uint8_t func3_bits[3];
                get_bits(bits, func3_bits, 12, 14);
                instruction_memory[instruction_index].func3 = convert_bits_to_num(func3_bits, 3, 0);

                //RS1 - bits 15-19
                uint8_t rs1_bits[5];
                get_bits(bits, rs1_bits, 15, 19);
                instruction_memory[instruction_index].rs1 = convert_bits_to_num(rs1_bits, 5, 0);

                //RS2 - bits 20-24
                uint8_t rs2_bits[5];
                get_bits(bits, rs2_bits, 20, 24);
                instruction_memory[instruction_index].rs2 = convert_bits_to_num(rs2_bits, 5, 0);

                //FUNC7 - bits 25-31
                uint8_t func7_bits[7];
                get_bits(bits, func7_bits, 25, 31);
                instruction_memory[instruction_index].func7 = convert_bits_to_num(func7_bits, 7, 0);

                switch (instruction_memory[instruction_index].func3) {
                    case 0:
                        if (instruction_memory[instruction_index].func7 == 0) {
                            instruction_memory[instruction_index].name=my_add;
                        } else if (instruction_memory[instruction_index].func7 == 32) {
                            instruction_memory[instruction_index].name=my_sub;
                        }
                        break;
                    
                    case 1:
                        instruction_memory[instruction_index].name=my_sll;
                        break;
                    
                    case 2:
                        instruction_memory[instruction_index].name=my_slt;
                        break;
                    
                    case 3:
                        instruction_memory[instruction_index].name=my_sltu;
                        break;
                    
                    case 4:
                        instruction_memory[instruction_index].name=my_xor;
                        break;
                    
                    case 5:
                        if (instruction_memory[instruction_index].func7 == 0) {
                            instruction_memory[instruction_index].name=my_srl;
                        } else if (instruction_memory[instruction_index].func7 == 32) {
                            instruction_memory[instruction_index].name=my_sra;
                        }
                        break;
                    
                    case 6:
                        instruction_memory[instruction_index].name=my_or;
                        break;
                    
                    case 7:
                        instruction_memory[instruction_index].name=my_and;
                        break;

                    default:
                        instruction_memory[instruction_index].type = UNKNOWN_TYPE;
                        instruction_memory[instruction_index].name = UNKNOWN_NAME;
                        break;
                    
                }

            }

            //-----------------------------
            //TYPE: S
            else if (opcode == 35) {
                instruction_memory[instruction_index].type = my_S;

                //FUNC3 - bits 12-14
                uint8_t func3_bits[3];
                get_bits(bits, func3_bits, 12, 14);
                instruction_memory[instruction_index].func3 = convert_bits_to_num(func3_bits, 3, 0);

                //RS1 - bits 15-19
                uint8_t rs1_bits[5];
                get_bits(bits, rs1_bits, 15, 19);
                instruction_memory[instruction_index].rs1 = convert_bits_to_num(rs1_bits, 5, 0);


                //RS2 - bits 20-24
                uint8_t rs2_bits[5];
                get_bits(bits, rs2_bits, 20, 24);
                instruction_memory[instruction_index].rs2 = convert_bits_to_num(rs2_bits, 5, 0);

                //getting the immediate
                uint8_t imm[] = {bits[31-31], bits[31-30], bits[31-29],bits[31-28],bits[31-27],bits[31-26],bits[31-25],bits[31-11],bits[31-10],bits[31-9],bits[31-8],bits[31-7]};
                

                instruction_memory[instruction_index].imm = convert_bits_to_num(imm, 12, 1);


                switch (instruction_memory[instruction_index].func3) {
                    case 0:
                        instruction_memory[instruction_index].name=my_sb;
                        break;
                    
                    case 1:
                        instruction_memory[instruction_index].name=my_sh;
                        break;
                    
                    case 2:
                        instruction_memory[instruction_index].name=my_sw;
                        break;

                    default:
                        instruction_memory[instruction_index].type = UNKNOWN_TYPE;
                        instruction_memory[instruction_index].name = UNKNOWN_NAME;
                        break;
                    
                }

            }

            //-----------------------------
            //TYPE: SB
            else if (opcode == 99) {
                instruction_memory[instruction_index].type = my_SB;

                //FUNC3 - bits 12-14
                uint8_t func3_bits[3];
                get_bits(bits, func3_bits, 12, 14);
                instruction_memory[instruction_index].func3 = convert_bits_to_num(func3_bits, 3, 0);

                //RS1 - bits 15-19
                uint8_t rs1_bits[5];
                get_bits(bits, rs1_bits, 15, 19);
                instruction_memory[instruction_index].rs1 = convert_bits_to_num(rs1_bits, 5, 0);


                //RS2 - bits 20-24
                uint8_t rs2_bits[5];
                get_bits(bits, rs2_bits, 20, 24);
                instruction_memory[instruction_index].rs2 = convert_bits_to_num(rs2_bits, 5, 0);

                //getting the immediate
                uint8_t imm[]= {bits[31-31], bits[31-7], bits[31-30], bits[31-29], 
                                bits[31-28], bits[31-27], bits[31-26], bits[31-25], 
                                bits[31-11], bits[31-10], bits[31-9], bits[31-8]};
                instruction_memory[instruction_index].imm = convert_bits_to_num(imm, 12, 1);

                switch (instruction_memory[instruction_index].func3) {
                    case 0:
                        instruction_memory[instruction_index].name=my_beq;
                        break;
                    
                    case 1:
                        instruction_memory[instruction_index].name=my_bne;
                        break;
                    
                    case 4:
                        instruction_memory[instruction_index].name=my_blt;
                        break;
                    
                    case 5:
                        instruction_memory[instruction_index].name=my_bge;
                        break;
                    
                    case 6:
                        instruction_memory[instruction_index].name=my_bltu;
                        break;
                    
                    case 7:
                        instruction_memory[instruction_index].name=my_bgeu;
                        break;

                    default:
                        instruction_memory[instruction_index].type = UNKNOWN_TYPE;
                        instruction_memory[instruction_index].name = UNKNOWN_NAME;
                        break;
                    
                }

            }

            //-----------------------------
            //TYPE: U
            else if (opcode == 55) {
                instruction_memory[instruction_index].type = my_U;
                instruction_memory[instruction_index].name = my_lui;


                //RD - bits 7-11
                uint8_t rd_bits[5];
                get_bits(bits, rd_bits, 7, 11);
                instruction_memory[instruction_index].rd = convert_bits_to_num(rd_bits, 5, 0);

                //getting the immediate  - bits 31-12
                uint8_t imm[32];

                int i = 0;
                while (i < 32) {
                    if (i < 20) {
                        imm[i] = bits[i];
                    } else {
                        imm[i] = 0;
                    }
                    i++;
                }

                instruction_memory[instruction_index].imm = convert_bits_to_num(imm, 32, 1);

                
            }

            else if (opcode == 111) {
                instruction_memory[instruction_index].type = my_UJ;
                instruction_memory[instruction_index].name = my_jal;

                //RD - bits 7-11
                uint8_t rd_bits[5];
                get_bits(bits, rd_bits, 7, 11);
                instruction_memory[instruction_index].rd = convert_bits_to_num(rd_bits, 5, 0);

                //immediate - bits 31-12, but in jumbled order
                uint8_t imm[]= {bits[31-31], bits[31-19], bits[31-18], bits[31-17], 
                                bits[31-16], bits[31-15], bits[31-14], bits[31-13], 
                                bits[31-12], bits[31-20], bits[31-30], bits[31-29], 
                                bits[31-28], bits[31-27], bits[31-26], bits[31-25], 
                                bits[31-24], bits[31-23], bits[31-22], bits[31-21]};


                instruction_memory[instruction_index].imm = convert_bits_to_num(imm, 20, 1);

            }

            else if (opcode == 0) {
                instruction_memory[instruction_index].type = EMPTY_TYPE;
                instruction_memory[instruction_index].name = EMPTY_NAME;
            }

            else {
                instruction_memory[instruction_index].type = UNKNOWN_TYPE;
                instruction_memory[instruction_index].name = UNKNOWN_NAME;
            }

            //also write the raw binary of the instruction into raw memory
            int i = 0;
            while (i < 4) {
                instruction_memory_raw[instruction_index*4 + i].data = line_buffer[i];
                i ++;
            }
            

            //check all the relevant register numbers given are between 0 and 31
            if (instruction_registers_are_valid(instruction_memory[instruction_index]) == false) {
                instruction_memory[instruction_index].type = INVALID_TYPE;
                instruction_memory[instruction_index].name = INVALID_NAME;
            }

            instruction_index ++;
            
        }   
        //this will only trigger once instruction index >=256, meaning we are in data memory now. 
        //this will read in the 4 bytes into the correponding 4 memory areas.
        else {
            int i = 0;
            while (i < 4) {
                data_memory[memory_index + i].data = line_buffer[i];
                i ++;
            }
            memory_index += 4;
        }
        
    }
    free(line_buffer);
    fclose(file);
    return 0;
}

//Based on the instruction type, this checks if the relevant registers are within range.
//Returns 0 if the registers aren't all valid
//Returns 1 if all registers are valid.
int instruction_registers_are_valid(struct instruction i) {
    if (i.type == my_R) {
        if (i.rs1 < 0 || i.rs1 > 31) {
            return 0;
        }
        if (i.rs2 < 0 || i.rs2 > 31) {
            return 0;
        }
        if (i.rd < 0 || i.rd > 31) {
            return 0;
        }

    } else if (i.type == my_I) {
        if (i.rs1 < 0 || i.rs1 > 31) {
            return 0;
        }
        if (i.rd < 0 || i.rd > 31) {
            return 0;
        }

    } else if (i.type == my_S || i.type == my_SB) {
        if (i.rs1 < 0 || i.rs1 > 31) {
            return 0;
        }
        if (i.rs2 < 0 || i.rs2 > 31) {
            return 0;
        }
    } 

    else if (i.type == my_U || i.type == my_UJ) {
        if (i.rd < 0 || i.rd > 31) {
            return 0;
        }
    } 
    return 1;
}