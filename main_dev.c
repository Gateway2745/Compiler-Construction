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
    Grammar g[100];
    readGrammar("grammar.txt",g);
    //printList(g[15].next);
    TermType t = {MULTIVAR_DEC};
    int num_rules;
    Term** rules = get_rules(g,t,&num_rules);
    printf("num rules = %d\n", num_rules);
    int i=0;
    while(i<num_rules)
    {
        printList(rules[i++]);
        printf("\n");
    }
}