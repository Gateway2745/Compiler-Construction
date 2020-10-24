#include "parser.h"

stackNode * pop(stack * s) {
    if(s->size == 0) return NULL;
    stackNode * temp = s->head;
    s->head = s->head->next;
    return temp;
}

stackNode * peek(stack * s) {
    return s->head;
}

void push(stack * s, stackNode * node) {
    (s->size)++;
    node->next = s->head;
    s->head = node;
}