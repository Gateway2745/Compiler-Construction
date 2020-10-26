#include "parser.h"

void driver(char * filename) {
    while (1)
    {
        int option;
        printf("Enter option: 0-exit, 1-create_tree 2-traverse_tree, 3-print_tree 4-print_table\n");
        scanf("%d", &option);
        if(!(option >= 0 && option <= 4)) {
            printf("Invalid option\n");
            continue;
        }

        if(option == 0) {
            printf("Exiting\n");
            break;
        }

        tokenStream Stream;
        parseTree Tree;
        Grammar G;

        readGrammar("grammar.txt", &G);
        tokeniseSourcecode(filename, &Stream);
        createParseTree(&Tree, &Stream, G);

        if(option == 1) {
            printf("Parse Tree created\n");
            continue;
        }

        typeExpressionTable Table;
        traverseParseTree(&Tree, &Table);

        if(option == 2) {
            printf("Parse Tree traversed\n");
            continue;
        }

        if(option == 3) {
            printParseTree(&Tree);
            continue;
        }

        if(option == 4) {
            printTypeExpressionTable(Table);
            continue;
        }
    }
    
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Proper usage - ./exec <input file>\n");
        exit(1);
    }
    driver(argv[1]);
    return 0;
}