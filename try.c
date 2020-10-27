#include "parser.h"

int get_length(Term * rule) {
	if(rule->next) return 1 + get_length(rule->next);
	return 1;
}

void reverse_dump(stack ** st, Term * rule) {
	stack * s = *st;

void tree_destroy(parseTree *root)  {
    if (num_children==0) {return;}
    for(int i = 0; i < root->num_children; i++){
    	tree_destroy(root->children[i]);
    	root->children[i] = NULL; //freeing the pointer of children to point to null
    }
	free(root);      
}

	if(s->head == NULL) s->head = (stackNode *) malloc(sizeof(stackNode));
	stackNode * node = s->head;
	while(node && node->next) node = node->next;




int try(cursor ** current_token_stream, tree, stack) {
int size = ? //malloc what size?
if(topstack->is_term ==1) {
	if(topstack->tok == tok_stream->[0])
		{pop(stack); 
			return;}
	else continue;

}  	// readterm does (topstack->tok == tok_stream->[0] )? 1:0 //pop from stack if success, else not
stackNode top_stack = peek(stack);
Term** rules = get_rules(G, top_stack->nonterm, &size);

for(Term * rule = NULL, *rule =  ){ //help pls
	temp_stack = stack(size, rule);
	cursor * temp_current_stream_token = *current_token_stream;
	Tree_setup (create only for non-terms)// sos

	while (temp_stack->head != NULL) {
		int is_term = peek(temp_stack)->is_term;
		if (is_term==1)  try(&temp_current_stream_token, tree->child[counter], temp_stack);
		if(!is_term) counter++;
		if(success == 0) break;
	}
	if(sucess){
		pop(stack);
		*Current_token_stream = temp_current_stream_token;
		break;
	}
	tree_destroy(tree);
	}
}


