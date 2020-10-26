#include "parser.h"

void internalPrintParseTree(parseTree * t, int depth) {
    while(t) {
        for(int i = 0; i < depth; i++) printf("\t");
        printf("\n");
        for(int i = 0; i < t->num_children; i++) internalPrintParseTree(t->children[i], depth+1);
    }
}

void printParseTree(parseTree * t) {
    internalPrintParseTree(t, 0);
}