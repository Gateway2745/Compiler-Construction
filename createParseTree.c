#include "parser.h"

int get_length(Term * rule) {
	if(!rule) return 0;
	return 1 + get_length(rule->next);
}

stackNode * reverse_dump(Term * rule) {
	if(!rule) return NULL;

	stackNode * node = (stackNode *) malloc(sizeof(stackNode));
	node->next = reverse_dump(rule->next);
	node->is_term = rule->is_term;
	node->type = rule->type;

	return node;
}

void print_rule_local(Term * rule) {
	if(!rule) return;
	char buf[25];
	get_str(rule->type, buf, rule->is_term);
	printf("%s (is_term %d) ", buf, rule->is_term);
	print_rule_local(rule->next);
}

void free_children(parseTree * t) {
	for(int i = 0; i < t->num_children; i++) free_children(t->children[i]);
	for(int i = 0; i < t->num_children; i++) free(t->children[i]);
	free(t->children);
}

int apply(Grammar * g, parseTree * t, tokenStream ** stream, Term * rule) {
	printf("In apply\n");

	tokenStream * s = *stream;
	char buffer[25];
	get_str(t->term.type, buffer, t->term.is_term);
	printf("Applying rule %d %s:\n", t->term.is_term, buffer);
	print_rule_local(rule);
	printf("\n");

	stack * local_stack = (stack *) malloc(sizeof(stack));
	local_stack->head = reverse_dump(rule);
	stackNode * mover = local_stack->head;

	int length = get_length(rule);
	local_stack->size = length;
	t->num_children = length;
	t->children = (parseTree **) malloc(length * sizeof(parseTree *));
	for(int i = 0; i < length; i++) t->children[i] = (parseTree *) malloc(sizeof(parseTree));

	printf("Reverse dump complete\n");
	
	for(int i = 0; i < length; i++) {

		stackNode * temp = pop(local_stack);
		// if(temp == NULL) printf("Underflow\n");		// Should not occur
		int is_term = temp->is_term;
		TermType type = temp->type;
		int error = 1;

		get_str(type, buffer, is_term);
		printf("%s\n", buffer);

		if(is_term) {
			error = (type.tok.token != s->token);
			if(error) return -1;
			t->children[i]->num_children = 0;
			t->children[i]->children = NULL;
			t->children[i]->term.is_term = 1;
			t->children[i]->term.type.tok.token = s->token;
			t->children[i]->term.type.tok.line_num = s->line_num;
			strcpy(t->children[i]->term.type.tok.lexeme, s->lexeme);
			s = s->next;
			continue;
		}

		printf("All good\n");

		int num_rules;
		Term ** rules = get_rules(g, type, &num_rules);
		for(int j = 0; j < num_rules; j++) {
			printf("I is %d, %p\n", i, t->children[i]);
			t->children[i]->term.is_term = 0;
			t->children[i]->term.type = type;
			printf("Sublooping %d\n", j);
			error = apply(g, t->children[i], &s, rules[j]);
			printf("Sublooping %d\n", j);
			if(!error) break;
			free_children(t);
		}
		if(error) return -1;
	}
	*stream = s;
	return 0;
}

void createParseTree(parseTree *t, tokenStream *s, Grammar g) {

	if(!t) {
		printf("Provide proper tree starter\n");
		return;
	}

	stackNode *starter = (stackNode *) malloc(sizeof(stackNode));
	starter->is_term = 0;
	starter->type.nt = PROGRAM;
	stack * st = (stack *) malloc(sizeof(stack));
	push(st, starter);

	t->term.is_term = 0;
	t->term.type.nt = PROGRAM;

	int num_rules;
	Term ** rules = get_rules(&g, starter->type, &num_rules);

	int error = apply(&g, t, &s, rules[0]);
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