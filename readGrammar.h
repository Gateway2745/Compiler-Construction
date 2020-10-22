typedef struct Term {
    char name[20];
    struct Term* next;
} Term;

typedef struct Grammar {
    char name[20];
    struct Term* next;
} Grammar;


