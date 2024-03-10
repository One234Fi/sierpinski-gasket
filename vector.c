#include "vector.h"
#include "vulkan.h"
#include <string.h>
#include <stdlib.h>

#define INITIAL_SIZE 8
#define GROWTH_RATE 2

void grow_vec(vec v) {
    uint32_t new_capacity = v->capacity * GROWTH_RATE;
    Vertex* new_data = malloc(sizeof(Vertex) * new_capacity);
    memcpy(new_data, v->data, sizeof(Vertex) * v->capacity);
    free(v->data);
    v->data = new_data;
    v->capacity = new_capacity;
}

vec init_vec() {
    vec v = malloc(sizeof(struct vector));
    v->data = malloc(sizeof(Vertex) * INITIAL_SIZE);
    v->size = 0;
    v->capacity = INITIAL_SIZE;

    return v;
}

void free_vec(vec v) {
    free(v->data);
    free(v);
}

void push(vec v, Vertex vert) {
    if (v->size >= v->capacity) {
        grow_vec(v);
    }
    v->data[v->size] = vert;
    v->size += 1;
}

Vertex get(vec v, uint32_t index) {
    return v->data[index];
}

