#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BANK_NUM 30
#define BANK_SIZE 64
#define TRUE 1
#define FALSE 0
#define HEAP_STARTING_ADDRESS 46848

struct node {
    int unused;
    int length;
    uint32_t virtual_address;
    uint8_t data[64];
    struct node* next;
};

//NOTE: heap structure
//the heap is store using a linked list. 
//The linked list is made up of nodes (representing memory banks) that store 64 bytes of data, 
//the virtual adddres of the bank, whether it is currently used, and the next node.
//When a chunk of banks are 'malloced', all of the nodes are set to used, and the first bank has 
//it's length set to the amount of banks in the chunk. All other banks keep the default length of -1.


struct node* create_heap();
int calculate_banks_needed(int size);
uint32_t my_malloc(struct node* head, int size);
int my_free(struct node* head, uint32_t virtual_address);
void* find_consecutive_banks(struct node* head, int banks_needed);
int set_to_used(struct node* head, int length);
int write_mem(struct node* head, uint32_t virtual_address, uint8_t data[], int length);
void free_linked_list(struct node* head);



struct node* create_heap() {
    uint32_t virtual_address = HEAP_STARTING_ADDRESS;

    //create the first node...
    struct node* head = NULL;
    head = (struct node*) malloc(sizeof(struct node));
    if (head == NULL) {
        return NULL;
    }

    head -> unused = TRUE; 
    head -> length = -1;
    head -> virtual_address = virtual_address;
    head -> next = NULL;
    
    struct node* current = head;

    int i = 0;

    //LOop through making nodes with the default values, linking them to each other  
    while (i < BANK_NUM-1) {
        struct node* n = (struct node*) malloc(sizeof(struct node));
        if (n == NULL) {
            return NULL;
        }

        virtual_address += 64;

        n -> unused = TRUE; 
        n -> length = -1;
        n -> virtual_address = virtual_address;
        n -> next = NULL;

        current -> next = n;
        current = current -> next;

        i ++;
    }
    return head;
}


// if this fails, it returns 0. Otherwise it returns the virtual adress
uint32_t my_malloc(struct node* head, int size) {
    int banks_needed = calculate_banks_needed(size);

    //find enough free consecutive banks to hold the memory needed
    struct node* node_start = find_consecutive_banks(head, banks_needed);

    node_start -> length = banks_needed;

    //goes through and sets all of these banks to used
    int failed = set_to_used(node_start, banks_needed);

    if (failed) {
        return 0;
    }

    return node_start -> virtual_address;

}


// returns 1 if any nodes were already set to used
int set_to_used(struct node* head, int length) {
    struct node* current = head;

    int i = 0;
    while (i < length) {
        if (current -> unused == FALSE) {
            return 1;
        }

        current -> unused = FALSE;
        current = current -> next;
        i ++;
    }
    return 0;
}

//if the size of the data is divisible by 64, then we will use n=size/64 banks exactly. 
//However if size/64 has some remainder, we will need an extra bank for that remainder.
int calculate_banks_needed(int size) {
    if (size % 64 == 0) {
        return size / 64;
    } else {
        return (size / 64 +1);
    }
}

void* find_consecutive_banks(struct node* head, int banks_needed) {
    struct node* current = head;

    int consec = 0;

    struct node* consec_start = NULL;

    while (current != NULL) {
        //if the current node is not unused, reset the count and move to the next one
        if (current -> unused == FALSE) {
            consec = 0;
            current = current -> next;
            consec_start = 0;
            continue;
        }

        //If the current bank is unused, check if we're already ion a run, and if we're not, set hte current node to be the start of a new run
        if (consec_start == NULL) {
            consec_start = current;
        }

        //Increment the size of the current run
        consec ++;

        if (consec == banks_needed) {
            return consec_start;
        }

        current = current -> next;
    }

    return NULL;
}

//returns 1 if the given memory was already free
int my_free(struct node* head, uint32_t virtual_address) {
    int list_position = (virtual_address - HEAP_STARTING_ADDRESS)/16;//might not be right

    struct node* current = head;

    //traverse to beginning of the memory block
    int i = 0;
    while (i < list_position) {
        current = current -> next;
        i ++;
    }

    int length = current -> length;

   //traverse through the memory block (based on length) and reset blocks
    i = 0;
    while (i < length) {
        if (current -> unused == TRUE) {
            return 1;
        }

        current -> unused = TRUE;
        current -> length = -1;

        current = current -> next;
        
        i ++;
        
    }

    return 0;
}

int write_mem(struct node* head, uint32_t virtual_address, uint8_t data[], int length) {
    int list_position = virtual_address - HEAP_STARTING_ADDRESS;

    struct node* current = head;

    //traverse to beginning of the memory block
    int i = 0;
    while (i < list_position) {
        current = current -> next;
        i ++;
    }

    int banks = calculate_banks_needed(length);

    int data_index = 0;
    int bank_index = 0;

    //split the data into blocks of 64, and start assigning them into the memory banks.
    while (bank_index < banks) {
        if (current -> unused == TRUE) {
            return 1;
        }
        data_index = 0;

        while (data_index < 64 && (bank_index*64+data_index)<length) {
            current -> data[data_index] = data[data_index];
            data_index += 1;
        }
        bank_index ++;
        current = current->next;
    }

    return 0;
}

//frees the memory being used to store the nodes. NOTE: this is different to 
//my_free, as it is freeing the actual data structure, not just setting nodes to unused.
void free_linked_list(struct node* head) {
    struct node* current = head;
    while (current != NULL) {
        struct node* next = current -> next;
        free(current);
        current = next;
    }
}
