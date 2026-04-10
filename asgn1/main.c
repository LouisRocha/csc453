#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define ORIGINAL_HUNK_SIZE 64000
#define ALIGNER_FACTOR 16

typedef struct chunk{
    size_t size;
    bool isFree;
    struct chunk *next;
} chunk;

static chunk *head = NULL;

chunk *traverse_chunks(size_t size) {
    chunk *current = head;

    while(current != NULL){
        if(current->isFree && current->size >= size){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

size_t byteAligner(size_t size){
    if (size % ALIGNER_FACTOR == 0){
        return size;
    }
    return size + (ALIGNER_FACTOR - (size % ALIGNER_FACTOR));
}

void *mymalloc(size_t size){
    if(size == 0){
        return NULL;
    }

    size = byteAligner(size); 

    chunk *potential_chunk = traverse_chunks(size);

    if(potential_chunk != NULL){
        chunk *remainder = (chunk *)((char *)(potential_chunk + 1) + size);
        remainder->size = potential_chunk->size - size - sizeof(chunk);
        remainder->isFree = true;
        remainder->next = potential_chunk->next;

        potential_chunk->size = size;
        potential_chunk->isFree = false;
        potential_chunk->next = remainder;

        return (void *)(potential_chunk + 1);

    } else {
        size_t og_hunk_size = ORIGINAL_HUNK_SIZE;
        if(sizeof(chunk) + size > og_hunk_size){
            og_hunk_size = size + sizeof(chunk);
        }

        void *start = sbrk(og_hunk_size);

        if(start == (void *) -1){
            errno = ENOMEM;
            return NULL;
        }

        chunk *new = (chunk *) start;
        new->size = og_hunk_size - sizeof(chunk);
        new->isFree = false;
        new->next = NULL;

        if(head == NULL){
            head = new;
        } else {
            chunk *current = head;
            while(current->next != NULL){
                current = current->next;
            }
            current->next = new;
        }
        return (void *)(new + 1);
    }
};

void myfree(void *ptr){
    chunk *ch = (chunk *) ptr - 1;
    ch->isFree = true;
}

int main(){
    printf("%p\n", sbrk(0));
    mymalloc(30);
    printf("%p\n", sbrk(0));
    mymalloc(100);
    printf("%p\n", sbrk(0));

    return 0;
}
