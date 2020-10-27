#include "parser.h"





int try(cursor ** current_token_stream, tree, stack) {
int size
if(topstack->is_term ==1) {
	if(topstack->tok == tok_stream->[0])
		pop(stack);
	else continue;

} return; 	// readterm does (topstack->tok == tok_stream->[0] )? 1:0 //pop from stack if success, else not
stackNode top_stack = peek(stack);
Term** rules = get_rules(G, top_stack->nonterm, &size);
for(Term * rule = NULL, *rule =  )
For rule in range size {
Push rule on temp_stack
Cursor * Temp_current_stream_token = *current_token_stream
Tree_setup (create only for non-terms)
While temp_stack {
Is_term = check_if_term(top_temp_stack)
Success = try(&temp_current_stream_token, tree->child[counter], temp_stack);
if(!is_term) counter++;
if(success == 0) break;
}
if(sucess){
Pop from stack
*Current_token_stream = Temp_current_stream_token
 Break
}
tree_destroy
}
}


