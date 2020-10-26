#include "parser.h"

link * copy_link(link * source) {
    link * new = (link *) malloc(sizeof(link));
    new->arr_info = source->arr_info;
    new->arr_storage = source->arr_storage;
    new->type = new->type;
    return new;
}

void add_info(parseTree * node, link * info, typeExpressionTable * table) {
    strcpy(info->id, node->children[0]->term.type.tok.lexeme);
    put_link(table, info);
    if(node->num_children > 1) add_info(node->children[1], copy_link(info), table);
}

int get_length_idx(parseTree * idx) {
    if(idx->num_children == 1 && idx->children[0]->term.type.nt == EPSILON) return 0;
    return 1 + get_length(idx->children[1]);
}

link * run_primitive(parseTree * node) {
    link * info = (link *) malloc(sizeof(link));
    info->arr_info = PRIMITIVE;
    info->arr_storage = NONE;
    if(node->children[0]->term.type.tok.token == KEY_INT) info->type.prim_info = INTEGER;
    else if(node->children[0]->term.type.tok.token == KEY_BOOL) info->type.prim_info = BOOLEAN;
    else if(node->children[0]->term.type.tok.token == KEY_REAL) info->type.prim_info = REAL;
    else printf("Error - Unknown primitive %s\n", node->children[0]->term.type.tok.lexeme);
    return info;
}

// Start of horrible sphagetti code

link * run_jagged(parseTree * dec, parseTree * init) {
    link * info = (link *) malloc(sizeof(link));
    info->arr_info = JAG_ARR;
    info->arr_storage = NONE;
    info->type.jagged_arr_info.betype = INTEGER;
    info->type.jagged_arr_info.num_dim = (dec->term.type.nt == JAGGED2_DEC) ? 2 : 3;
    int dim2 = (info->type.jagged_arr_info.num_dim == 2);
    int x, y;
    parseTree * x_node = dec->children[3]->children[0]->children[0];
    parseTree * y_node = dec->children[3]->children[2]->children[0];

    if(x_node->term.type.tok.token == ID || y_node->term.type.tok.token == ID) {
        printf("Error - Line %d - Jagged arrays cannot have dynamic bounds\n", x_node->term.type.tok.line_num);
        return NULL;
    }
    info->type.jagged_arr_info.range_R1.r1 = atoi(x_node->term.type.tok.lexeme);
    info->type.jagged_arr_info.range_R1.r2 = atoi(y_node->term.type.tok.lexeme);
    if(info->type.jagged_arr_info.range_R1.r1 > info->type.jagged_arr_info.range_R1.r2) {
        printf("Error - Line %d - Lower bound cannot exceed upper bound\n", x_node->term.type.tok.line_num);
        return NULL;
    }

    int range_1 = 1 + info->type.jagged_arr_info.range_R1.r2 - info->type.jagged_arr_info.range_R1.r1;
    info->type.jagged_arr_info.range_R2 = (rng_R2 *) malloc(range_1 * sizeof(rng_R2));
    
    if(dim2) {
        for(int i = 0; i < range_1; i++) {
            info->type.jagged_arr_info.range_R2[i].num_dim = 1;
            info->type.jagged_arr_info.range_R2[i].dims = (int *) malloc(sizeof(int));
        }
        int count = 0;
        while(count < range_1 && init->children[0]->term.is_term == 0 && init->children[0]->term.type.nt == JAGGED2_SINGLE_INIT) {
            info->type.jagged_arr_info.range_R2[count++].dims[0] = atoi(init->children[0]->children[6]->term.type.tok.lexeme);
            if(init->num_children > 1) init = init->children[1];
            else break;
        }
        if(count < range_1 || init->num_children > 1) {
            printf("Error - Line %d - Size mismatch\n", init->children[0]->children[6]->term.type.tok.line_num);
            return NULL;
        }
        return info;
    }
    
    for(int i = 0; i < range_1; i++) {
        int num_dim = atoi(init->children[0]->children[6]->term.type.tok.lexeme);
        info->type.jagged_arr_info.range_R2[i].num_dim = num_dim;
        info->type.jagged_arr_info.range_R2[i].dims = (int *) malloc(num_dim * sizeof(int));
        parseTree * range = init->children[0]->children[10];

        for(int j = 0; j < num_dim; j++) {
            int length = get_length_idx(range->children[0]);
            if(length == 0) {
                printf("Error - Line %d - Detected 0 length subrange\n", init->children[0]->children[6]->term.type.tok.line_num);
                return NULL;
            }
            info->type.jagged_arr_info.range_R2[i].dims[j] = length;
            if(range->num_children < 3 && (j < num_dim - 1)) {
                printf("Error - Line %d - Size mismatch\n", range->children[0]->children[0]->term.type.tok.line_num);
                return NULL;
            }
        }
        if(init->num_children < 2 && (i < range_1 - 1) || range->num_children > 1) {
            printf("Error - Line %d - Size mismatch\n", init->children[0]->children[6]->term.type.tok.line_num);
            return NULL;
        }
        init = init->children[1];
    }
    if(init->num_children > 1) {
        printf("Error - Line %d - Size mismatch\n", init->children[0]->children[6]->term.type.tok.line_num);
        return NULL;
    }
    return info;
}

// Horrible sphagetti code <hopefully> ends here

link * get_type(parseTree * tree, int is_single) {
    int dec = (is_single) ? 3 : 6;
    if(tree->children[dec]->children[0]->term.is_term == 0) return run_primitive(tree->children[dec]->children[0]);
    else if(tree->children[dec]->children[0]->term.type.tok.token == KEY_ARR) return run_rect(tree->children[dec]);
    else return run_jagged(tree->children[dec], tree->children[dec+2]);
}

void traverseDeclares(parseTree * tree, typeExpressionTable * table) {
    if(!(tree->term.is_term == 0 && tree->term.type.nt == DECL_STMT)) {
        for(int i = 0; i < tree->num_children; i++) traverseDeclares(tree->children[i], table);
        return;
    }
    int single = (tree->children[0]->term.type.nt == SINGLEVAR_DEC);
    tree = tree->children[0];

    link * info = get_type(tree, single);
    if(single) {
        strcpy(info->id, tree->children[1]->term.type.tok.lexeme);
        put_link(table, info);
    }
    else add_info(tree->children[4], info, table);
}


void traverseAssigns(parseTree * tree, typeExpressionTable * table) {
    if(!(tree->term.is_term == 0 && tree->term.type.nt == ASSGN_STMT)) {
        for(int i = 0; i < tree->num_children; i++) traverseAssigns(tree->children[i], table);
        return;
    }
    

}


void traverseParseTree(parseTree *t, typeExpressionTable *Table) {
    traverseDeclares(t, Table);
    traverseAssigns(t, Table);
}