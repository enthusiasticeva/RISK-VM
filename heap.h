#ifndef my_heap   /* Include guard */
#define my_heap

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BANK_NUM 128
#define BANK_SIZE 64
#define TRUE 1
#define FALSE 0
#define HEAP_STARTING_ADDRESS 46848

struct node {
    int unused;
    int length;
    // void* data;
    uint32_t virtual_address;
    uint8_t data[64];
    struct node* next;
};


struct node* create_heap();
int calculate_banks_needed(int size);
uint32_t my_malloc(struct node* head, int size);
int my_free(struct node* head, uint32_t virtual_address);
void* find_consecutive_banks(struct node* head, int banks_needed);
int set_to_used(struct node* head, int length);
int write_mem(struct node* head, uint32_t virtual_address, uint8_t data[], int length);
void free_linked_list(struct node* head);


#endif // FOO_H_