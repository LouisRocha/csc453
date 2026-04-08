#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define ORIGINAL_HUNK_SIZE 64000

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
    if (size % 16 == 0){
        return size;
    }
    return size + (16 - (size % 16));
}

void *mymalloc(size_t size){
    if(size == 0){
        return NULL;
    }

    size = byteAligner(size); 

    chunk *potential_chunk = traverse_chunks(size);

    if(potential_chunk != NULL){
        chunk *remainder  

        potential_chunk->isFree = false;
        return (void *)(potential_chunk + 1);
    } else {
        void *start = sbrk(ORIGINAL_HUNK_SIZE);

        if(start == (void *) -1){
            errno = ENOMEM;
            return NULL;
        }

        chunk *new = (chunk *) start;
        new->size = ORIGINAL_HUNK_SIZE - sizeof(chunk);
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

int main(){
    printf("%p\n", sbrk(0));
    mymalloc(30);
    printf("%p\n", sbrk(0));
    mymalloc(100);
    printf("%p\n", sbrk(0));

    return 0;
}
