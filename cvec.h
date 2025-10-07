// MIT License
// 
// Copyright (c) 2025 George Theodorakis
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CVEC_H_
#define CVEC_H_

#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Various primitives to use */
typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// Internal memory management funcs
#ifndef INTERNAL_CVEC_FREE
#define INTERNAL_CVEC_FREE free
#endif
#ifndef INTERNAL_CVEC_MALLOC
#define INTERNAL_CVEC_MALLOC malloc
#endif
#ifndef INTERNAL_CVEC_CALLOC
#define INTERNAL_CVEC_CALLOC calloc
#endif
#ifndef INTERNAL_CVEC_REALLOC
#define INTERNAL_CVEC_REALLOC realloc
#endif

#define INTERNAL_CVEC_GET_METADATA(vec)\
    ((_internal_cvec_metadata*)((uint8_t*)(vec) - sizeof(_internal_cvec_metadata)))


// typedef of destructor
typedef void (*cvec_elem_destructor)(void* elem);

typedef struct
{
    uint64 size;        
    uint64 capacity;
    uint64 typesize;

    cvec_elem_destructor destructor;
} _internal_cvec_metadata ;


// Returns the vector's capacity
static inline 
uint64
cvec_get_capacity(const void* vec)
{
    return vec ? (INTERNAL_CVEC_GET_METADATA(vec)->capacity) : (0);
}



// Returns the vector's size
static inline 
uint64
cvec_get_sz(const void* vec)
{
    return vec ? (INTERNAL_CVEC_GET_METADATA(vec)->size) : (0);
}



// Returns the vector's type size
static inline 
uint64
cvec_get_type_sz(const void* vec)
{
    return vec ? (INTERNAL_CVEC_GET_METADATA(vec)->typesize) : (0);
}


// Returns whether the function is empty or not
static inline
int32 cvec_empty(const void* vec)
{
    return (cvec_get_sz(vec) == 0);
}

// Returns a vector with reserved space
static inline
void* cvec_reserve(size_t capacity, 
                   size_t elem_sz,
                   cvec_elem_destructor destructor)
{
    uint64 new_allocated_size__  = sizeof(_internal_cvec_metadata) + capacity * elem_sz;

    void* vec__ = INTERNAL_CVEC_MALLOC(new_allocated_size__);
    memset(vec__, 0, new_allocated_size__);

    _internal_cvec_metadata* meta__ = (_internal_cvec_metadata*) vec__;

    meta__->capacity   = capacity;
    meta__->typesize   = elem_sz;
    meta__->destructor = destructor;

    return ((uint8*)vec__ + sizeof(_internal_cvec_metadata));
}

// Returns a vector with capacity of 1
static inline
void* cvec_create(size_t elem_sz,
                  cvec_elem_destructor destructor)
{
    return cvec_reserve(1,elem_sz,destructor);
}


// Push back an element in the vector
static inline
void* cvec_push_back(void* vec , const void* elem)
{
    if(!vec)
        return (NULL);

    _internal_cvec_metadata* meta__ = INTERNAL_CVEC_GET_METADATA(vec);

    uint64               sz         = meta__->size;
    uint64               capacity   = meta__->capacity;
    uint64               elem_sz    = meta__->typesize;
    cvec_elem_destructor destructor = meta__->destructor;

    if(capacity == sz)
    {
        uint64 new_capacity = (capacity == 0) ? 1 : capacity*2;
        meta__ = (_internal_cvec_metadata*)
                 INTERNAL_CVEC_REALLOC(meta__,
                                        sizeof(_internal_cvec_metadata) + new_capacity * elem_sz);

        // reset the memory to 0 after a point
        vec = meta__ + 1;
        memset((uint8*)vec + capacity, 0, (new_capacity - capacity) * elem_sz);

        meta__->capacity = new_capacity;
    }

    memcpy((uint8*)vec + (sz * elem_sz), elem, elem_sz);
    meta__->size++;

    return vec;
}


// Erase an element from the vector
static inline
void cvec_erase(void* vec, size_t index)
{    
    if(!vec) return ;

    _internal_cvec_metadata* meta__ = INTERNAL_CVEC_GET_METADATA(vec);

    uint64               sz         = meta__->size;
    uint64               capacity   = meta__->capacity;
    uint64               elem_sz    = meta__->typesize;
    cvec_elem_destructor destructor = meta__->destructor;

    uint8_t* data = (uint8_t*)vec;

    // If out of bounds, do not erase anything
    if(index >= sz) return ;

    if (destructor) {
        destructor(data + index * elem_sz);
    }

    // Shift elements
    memmove(data + index * elem_sz,
            data + (index + 1)  * elem_sz,
            (sz -  (index + 1)) * elem_sz);

    meta__->size--;
    
    return ;
}

// Free the vector and its metadata
static inline
void cvec_free(void* vec)
{
    if(!vec) return ;

    _internal_cvec_metadata* meta__ = INTERNAL_CVEC_GET_METADATA(vec);

    uint64               sz         = meta__->size;
    uint64               elem_sz    = meta__->typesize;
    cvec_elem_destructor destructor = meta__->destructor;

    if(destructor)
    {
        uint8* data = (uint8*) vec;

        for(uint64 i = 0; i < sz; i++)
            destructor(data + i*elem_sz);
    }

    INTERNAL_CVEC_FREE(meta__);
    return ;
}


// undef internal cvec stuff
#undef INTERNAL_CVEC_FREE
#undef INTERNAL_CVEC_MALLOC
#undef INTERNAL_CVEC_CALLOC
#undef INTERNAL_CVEC_REALLOC

#undef INTERNAL_CVEC_GET_METADATA

#endif //CVEC_H_

