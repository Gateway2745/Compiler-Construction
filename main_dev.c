#include "parser.h"

void printList(Term* head)
{
    while(head)
    {
        if(head->is_term==0) printf("%d -> ", head->type.nt);
        else printf("%d -> ", head->type.tok.token);
        
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
    for(int i=0;i<G.num_rules;i++) {
        printf("%d\n", G.rules[i]->is_term);
    }
    Term** rules = get_rules(&G,t, &num_rules);
    printf("num rules = %d\n", num_rules);
    int i=0;
    while(i<num_rules)
    {
        printList(rules[i++]);
        printf("\n");
    }
}