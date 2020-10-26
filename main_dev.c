#include "parser.h"

void printList(Term* head)
{
    char buf[25];
    while(head)
    {
        get_str(head->type, buf, head->is_term);
        printf("%s -> ", buf);
        head = head->next;
    }
}

int main()
{
    Grammar G;
    printf("Start\n");
    readGrammar("grammar.txt", &G);
    //printList(g[15].next);
    TermType t;
    t.nt = MULTIVAR_DEC;
    int num_rules;
    printf("Hi %d\n", G.num_rules);
    Term** rules = get_rules(&G,t, &num_rules);
    printf("num rules = %d\n", num_rules);
    int i=0;
    while(i<num_rules)
    {
        printList(rules[i++]);
        printf("\n");
    }

    tokenStream s;
    tokeniseSourcecode("t2.txt", &s);
    tokenStream * mover = &s;
    while (mover)
    {
        printf("%d %s ", mover->line_num, mover->lexeme);
        printf("%d\n", mover->token);
        mover = mover->next;
    }
}