#include "parser.h"

int is_whitespace(char * source) {
    if(*source == ' ' || *source == '\t' || *source == '\v' || *source == '\f' || *source == '\r') return 1;
    return 0;
}

void tokeniseSourcecode(char * source_file, tokenStream *s) {
    long size;
    char * source = loadfile(source_file, &size);

    int line_num = 1;
    int read = 0;
    char buf[21];
    buf[20] = '\0';
    int temp;

    if(s==NULL) {
        printf("Provide proper tokenStream head\n");
        return -1;
    }

    while(*source) {

        if(*source == '\n') {
            line_num++;
            continue;
        }
        if(is_whitespace(source)) {
            source++;
            continue;
        }
        read = 0;
        temp = 0;
        while(source[temp] && source[temp] != '\n' && !is_whitespace(source + temp) && temp < 20) {
            buf[temp] = source[temp];
            temp++;
        }
        buf[temp] = '\0';
        if(temp == 20) {
            printf("Aborting - Lexeme %s... at line %d too long\n", buf, line_num);
            exit(0);
        }

        read_single_symbol(buf, s, &read);
        read_multi_symbol(buf, s, &read);
        read_number(buf, s, &read);
        read_keyword(buf, s, &read);
        read_id(buf, s, &read);

        if(read == 0) {
            printf("Aborting - Token %s at line %d not recognised\n", buf, line_num);
            exit(0);
        }
        source += read;
        s->next = (tokenStream *) malloc(sizeof(tokenStream));
    }

    s->token = END;
    s->lexeme = NULL;
    s->line_num = line_num;
    s->next = NULL;
}