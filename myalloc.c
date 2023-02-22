#include "myalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

struct block *head = NULL;  // Head of the list, empty

void *myalloc(int size) {
    if (head == NULL) {
        head = mmap(NULL, 1024, PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_PRIVATE, -1, 0);
        head->next = NULL;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
    }

    struct block *curr = head;

    while(curr != NULL) {

        int padded_requested_size = PADDED_SIZE(size);

        if (curr -> in_use == 0 && curr -> size >= padded_requested_size) {

            int padded_block_size = PADDED_SIZE(sizeof(struct block));
            int required_space = 16 + padded_block_size + padded_requested_size;

            if (curr -> size >= required_space) { //split block function

                void* newblockptr = curr;
                newblockptr = newblockptr + padded_requested_size;

                struct block *newblock = newblockptr;  
                newblock -> next = curr -> next;
                newblock -> size = curr -> size - padded_requested_size - padded_block_size;
                newblock -> in_use = 0;

                curr -> size = padded_requested_size;
                curr -> next = newblock;
            }

            curr -> in_use = 1;
            return PTR_OFFSET(curr, padded_block_size);
        }

        curr = curr->next;
    }
    return NULL;

}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

void myfree(void *p) {
    void *blockp = p - PADDED_SIZE(sizeof(struct block));
    ((struct block *) blockp) -> in_use = 0;
}

int main() {
    void *p;

    myalloc(10);     print_data();
    p = myalloc(20); print_data();
    myalloc(30);     print_data();
    myfree(p);       print_data();
    myalloc(40);     print_data();
    myalloc(10);     print_data();
}