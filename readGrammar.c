#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "readGrammar.h"

void printList(Term* head)
{
    while(head)
    {
        printf("%s\n", head->name);
        head = head->next;
    }
}

char* removeFirstAndLast(char* s)
{
    s++;
    s[strlen(s)-1]='\0';
    return s;
}

Term* addNode(Term* head, char* word) // inserts node at end of linked list
{
    Term* last = head;
    Term* new = (Term*) malloc(sizeof(Term));
    strcpy(new->name,word);
    if(!head) return new;
    while(last->next) last=last->next;
    last->next = new;
    return head;
}


Grammar* readGrammar(char *filename, Grammar* g)
{
    int linecount=0;
    FILE *text = fopen(filename, "r");
    char line[150];
    while (fgets(line, 150, text) != NULL)
    {
        if(line[0]=='\n') continue;
        // printf("%s", line);

        char *token;
        token = strtok(line, " ");
        token = removeFirstAndLast(token); // strip '<' and '>'
        strcpy(g[linecount].name,token);
        g[linecount].next=NULL;
        token = strtok(NULL, " ");
        //printf("\n");
        while(token != NULL) 
        {
            token[strcspn(token, "\n")] = 0;
            //printf("%s,", token);
            token = removeFirstAndLast(token);
            g[linecount].next = addNode(g[linecount].next, token);
            token = strtok(NULL, " ");
        }
        linecount++;
    }

}

int main()
{
    Grammar g[100];
    readGrammar("grammar.txt",g);
    printList(g[0].next);
}