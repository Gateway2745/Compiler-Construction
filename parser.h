#ifndef PARSER_H
#define PARSER_H

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

typedef enum NonTerm {
    PROGRAM, DECL_LIST, ASSGN_LIST, DECL_STMT, ASSGN_STMT,
    INDEX, RNG, TYPE, ARRAY_RANGE, DTYPE,
    ONE_OR_MORE_VAR, LISTVAR, SINGLEVAR_DEC, MULTIVAR_DEC,
    JAGGED2_DEC, JAGGED3_DEC, JAGGED2_INIT, JAGGED3_INIT,
    JAGGED2_IDX, JAGGED3_IDX, JAGGED2_RNG, JAGGED3_RNG,
    JAGGED2_SINGLE_INIT, JAGGED3_SINGLE_INIT,
    EXPRESSION, ARRAY_SEL, VAR, ARRAY_IDX,
    ARITHMETIC_EXP, BOOLEAN_EXP,
    TERM, FACTOR, L_OR_TERM, L_AND_TERM,
    EPSILON
} NonTerm;

typedef enum Token {
    BO, BC, SQO, SQC, CURLO, CURLC, CLN, SEMCOL, ROP, ASSGN,
    KEY_PROG, KEY_DECL, KEY_LIST, KEY_OF, KEY_VARS, KEY_ARR, R1,
    KEY_SIZE, KEY_VALS, KEY_JAG, KEY_INT, KEY_REAL, KEY_BOOL,
    OP_PLUS, OP_MINUS, OP_STAR, OP_SLASH, OP_AND, OP_OR,
    ID, INT,
    END
} Token;

typedef struct tokenStream {
    Token token;
    char lexeme[21];
    int line_num;
    struct tokenStream * next;
} tokenStream;

typedef union TermType {
    tokenStream tok;
    NonTerm nt;
} TermType;

typedef struct Term {
    struct Term* next;
    int is_term;
    TermType type;
} Term;

typedef struct Grammar {
    Term **rules;
    int num_rules;
} Grammar;

typedef struct parseTree {
    Term term;
    int num_children;
    struct parseTree ** children;
    //type stuff, later
} parseTree;

typedef struct stackNode {
    struct stackNode * next;
    int is_term;
    TermType type;
    char val[21];
} stackNode;

typedef struct stack {
    int size;
    stackNode * head;
} stack;

typedef struct type_exp {
    ;
} type_exp;

typedef struct link {
    link * next;
    link * prev;
    char id[21];                                                // Field 1
    enum arr_info {PRIMITIVE, RECT_ARR, JAG_ARR} arr_info;      // Field 2
    enum arr_storage {STATIC, DYNAMIC, NONE} arr_storage;       // Field 3
    type_exp type;                                              // Field 4
} link;

typedef struct typeExpressionTable {
    link * heads[16];
} typeExpressionTable;

char * loadfile (char * filename, long * sizeptr);

link * get_link(typeExpressionTable * t, char * str);
void put_link(typeExpressionTable * t, link * l);

void tokeniseSourcecode(char * source_file, tokenStream *s);
void readGrammar(char *filename, Grammar* g);
void createParseTree(parseTree *t, tokenStream *s, Grammar g);
void traverseParseTree(parseTree *t, typeExpressionTable *Table);
void printParseTree(parseTree * t);
void printTypeExpressionTable(typeExpressionTable T);

void get_str(TermType t, char * buf, int is_term);
Term** get_rules(Grammar* g, TermType t, int* num_rules);

stackNode * pop(stack * s);
stackNode * peek(stack * s);
void push(stack * s, stackNode * node);

#endif