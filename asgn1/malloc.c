#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

/*
 * malloc.c - implementation of malloc, free, calloc, and realloc
 */

#define ORIGINAL_HUNK_SIZE 64000 /* size of each sbrk() req. */
#define ALIGNER_FACTOR 16        /* allocations aligned to X bytes (16 in this case)*/

typedef struct chunk{
    size_t size;
    bool isFree;
    struct chunk *next;
} chunk;

static chunk *head = NULL; /* Head of the linked list of chunks */

/*
 * traverse_chunks - Search the chunk list for a free chunk large enough
 * based on given size.
 * Returns a pointer to the first suitable chunk, or NULL if none found.
 */
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

/*
 * byteAligner - Round "size" up to the nearest multiple of ALIGNER_FACTOR.
 * This ensures all returned pointers are 16-byte aligned.
 */
size_t byteAligner(size_t size){
    if (size % ALIGNER_FACTOR == 0){
        return size;
    }
    return size + (ALIGNER_FACTOR - (size % ALIGNER_FACTOR));
}
/*
 * malloc - Allocate "size" bytes of memory.
 * First searches the free list for a suitable chunk. If none found,
 * asks for more memory from the OS with sbrk(2).
 * Returns pointer to allocated memory, or NULL on failure.
 */
void *malloc(size_t size){
    if(size == 0){
        return NULL;
    }

    size = byteAligner(size); 

    chunk *potential_chunk = traverse_chunks(size);
    void *result;

    if(potential_chunk != NULL){
        /* Split the free chunk into used + remainder to avoid wasting memory */
        chunk *remainder = (chunk *)((char *)(potential_chunk + 1) + size);
        remainder->size = potential_chunk->size - size - sizeof(chunk);
        remainder->isFree = true;
        remainder->next = potential_chunk->next;

        potential_chunk->size = size;
        potential_chunk->isFree = false;
        potential_chunk->next = remainder;

        result = (void *)(potential_chunk + 1);

    } else {
        /* No suitable chunk found, request more memory from OS */
        size_t og_hunk_size = ORIGINAL_HUNK_SIZE;
        if(sizeof(chunk) + size > og_hunk_size){
            /* Request is larger than default hunk size, allocate exactly what's needed */
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

        /* Append new chunk to end of list, or set as head if list is empty */
        if(head == NULL){
            head = new;
        } else {
            chunk *current = head;
            while(current->next != NULL){
                current = current->next;
            }
            current->next = new;
        }
        result = (void *)(new + 1);
    }

    if(getenv("DEBUG_MALLOC")){
        char buf[256];
        snprintf(buf, sizeof(buf), "MALLOC: malloc(%zu) => (ptr=%p, size=%zu)\n", size, result, size);
        write(2, buf, strlen(buf));
    }
    
    return result;
};

/*
 * free - Mark the chunk at ptr as free and coalesce adjacent free chunks
 * to reduce fragmentation.
 * Does nothing if ptr is NULL.
 */
void free(void *ptr){
    if(ptr == NULL){
        return;
    }
    
    if(getenv("DEBUG_MALLOC")){
        char buf[256];
        snprintf(buf, sizeof(buf), "MALLOC: free(%p)\n", ptr);
        write(2, buf, strlen(buf));
    }

    /* Walk back one header to get the chunk metadata */
    chunk *ch = (chunk *) ptr - 1;
    ch->isFree = true;

    /* Coalesce adjacent free chunks to prevent fragmentation */
    chunk *current = head;
    while(current != NULL){
        if(current->isFree){
            /* Merge with next chunk while it is also free */
            while(current->next != NULL && current->next->isFree){
                current->size += sizeof(chunk) + current->next->size;
                current->next = current->next->next;
            }
        }
        current = current->next;
    }
}

/*
 * calloc - Allocate memory for an array of num elements of "size" bytes each.
 * Memory is zeroed (not garbage values) before returning.
 * Returns pointer to allocated memory, or NULL on failure.
 */
void *calloc(size_t num, size_t size){
    void *result = malloc(num * size);

    if (result == NULL){
        return NULL;
    }

    /* Zero out the allocated memory */
    memset(result, 0, num * size);

    if(getenv("DEBUG_MALLOC")){
        char buf[256];
        snprintf(buf, sizeof(buf), "MALLOC: calloc(%zu,%zu) => (ptr=%p, size=%zu)\n", num, size, result, num * size);
        write(2, buf, strlen(buf));
    }

    return result;
}

/*
 * realloc - Resize the allocation at ptr to "size" bytes.
 * Attempts in-place expansion by merging with adjacent free chunk.
 * Falls back to allocating new memory and copying if needed.
 * If ptr is NULL, behaves like malloc. If size is 0, behaves like free.
 * Returns pointer to resized memory, or NULL if allocation fails
 */
void *realloc(void *ptr, size_t size){
    if(ptr == NULL){
        return malloc(size);
    }
    if(size == 0){
        free(ptr);
        return NULL;
    }

    chunk *ch = (chunk *)ptr - 1;
    void *result;

    if(size < ch->size){
        /* Shrinking: split off the unused portion as a new free chunk */
        chunk *remainder = (chunk *)((char *)(ch + 1) + size);
        remainder->size = ch->size - size - sizeof(chunk);
        remainder->isFree = true;
        remainder->next = ch->next;

        ch->size = size;
        ch->isFree = false;
        ch->next = remainder;

        result = ptr;
    }
    else if(size == ch->size){
        /* Same size, nothing to do */
        result = ptr;
    }
    else {
        /* Growing: first try to expand in place by merging with next chunk */
        if(ch->next != NULL && ch->next->isFree &&
        ch->size + sizeof(chunk) + ch->next->size >= size){

            ch->size += sizeof(chunk) + ch->next->size;
            ch->next = ch->next->next;
            result = ptr;

        } else {
            /* In-place expansion failed, allocate new block and copy data */
            void *alternative = malloc(size);
            if(alternative == NULL){
                return NULL;
            }

            memcpy(alternative, ptr, ch->size);
            free(ptr);
            result = alternative;
        }
    }

    if(getenv("DEBUG_MALLOC")){
        char buf[256];
        snprintf(buf, sizeof(buf), "MALLOC: realloc(%p,%zu) => (ptr=%p, size=%zu)\n", ptr, size, result, size);
        write(2, buf, strlen(buf));
    }
    
    return result;
}
