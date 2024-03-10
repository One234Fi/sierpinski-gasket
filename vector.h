#ifndef VECTOR_H
#define VECTOR_H

#include "vulkan.h"
#include <stdint.h>

struct vector {
    Vertex* data;
    uint32_t size;
    uint32_t capacity;
};
typedef struct vector* vec;

vec init_vec();
void push(vec v, Vertex vert);
Vertex get(vec v, uint32_t index);
void free_vec(vec v);

#endif
