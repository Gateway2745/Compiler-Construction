#include "parser.h"

int get_length(Term * rule) {
	if(rule->next) return 1 + get_length(rule->next);
	return 1;
}

void reverse_dump(stack ** st, Term * rule) {
	stack * s = *st;

	if(!rule) return;

	if(s->head == NULL) s->head = (stackNode *) malloc(sizeof(stackNode));
	stackNode * node = s->head;
	while(node && node->next) node = node->next;

	node->next = NULL;
	node->is_term = rule->is_term;
	node->type = rule->type;

	reverse_dump(st, rule->next);
}

int apply(Grammar * g, parseTree * t, tokenStream * s, Term * rule) {

	stack * local_stack = (stack *) malloc(sizeof(stack));
	reverse_dump(&local_stack, rule);
	int length = get_length(rule);
	t->num_children = length;
	t->children = (parseTree **) malloc(length * sizeof(parseTree *));
	
	for(int i = 0; i < length; i++) {
		t->children[i] = (parseTree *) malloc(sizeof(parseTree));
		int is_term = local_stack->head->is_term;
		TermType type = pop(local_stack)->type;

		if(is_term) {
			int error = (type.tok.token != s->token);
			if(error) return -1;
			t->num_children = 0;
			t->children = NULL;
			t->term.is_term = 1;
			t->term.type.tok.token = type.tok.token;
			t->term.type.tok.line_num = type.tok.line_num;
			strcpy(t->term.type.tok.lexeme, type.tok.lexeme);
		}

		int num_rules;
		Term ** rules = get_rules(g, type, &num_rules);
		for(int j = 0; j < num_rules; j++) {
			;
		}
	}
}

void createParseTree(parseTree *t, tokenStream *s, Grammar g) {

	if(!t) {
		printf("Provide proper tree starter\n");
		return;
	}

	stackNode *starter = (stackNode *) malloc(sizeof(stackNode));
	starter->is_term = 0;
	starter->type.nt = KEY_PROG;
	stack * st = (stack *) malloc(sizeof(stack));
	push(st, starter);

	int num_rules;
	Term ** rules = get_rules(&g, starter->type, &num_rules);

	int error = apply(&g, t, s, rules[0]);
	if(error) {
		printf("Failed to build parse tree\n");
		exit(1);
	}
}




/*
int try(cursor ** current_token_stream, tree, stack) {
int size
if(topstack->is_term ==1) {
	if(topstack->tok == tok_stream->[0])
		pop(stack);
	else continue;

} return; 	// readterm does (topstack->tok == tok_stream->[0] )? 1:0 //pop from stack if success, else not
stackNode top_stack = peek(stack);
Term** rules = get_rules(G, top_stack->nonterm, &size);
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
*/