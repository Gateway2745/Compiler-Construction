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

int get_depth(parseTree * node) {
    if(node->num_children < 4) return 1;
    else return 1 + get_depth(node->children[3]);
}

int fill_ranges(parseTree * node, Var_Pair * pairs, typeExpressionTable * table) {
    parseTree * lower = node->children[1]->children[0]->children[0];
    parseTree * higher = node->children[1]->children[2]->children[0];
    if(lower->term.type.tok.token == ID) {
        link * bound = get_link(table, lower->term.type.tok.lexeme);
        if(!bound) {
            printf("Error - Line %d - Identifier not declared\n", lower->term.type.tok.line_num);
            return -1;
        }
        if(!(bound->arr_info == PRIMITIVE && bound->type.prim_info == INTEGER)) {
            printf("Error - Line %d - Non integer bounds not allowed\n", lower->term.type.tok.line_num);
            return -1;
        }
        pairs[0].is_r1_static = 0;
        strcpy(pairs[0].r1.r_d, lower->term.type.tok.lexeme);
    }
    else {
        pairs[0].is_r1_static = 1;
        pairs[0].r1.r_s = atoi(lower->term.type.tok.lexeme);
    }
    if(higher->term.type.tok.token == ID) {
        link * bound = get_link(table, higher->term.type.tok.lexeme);
        if(!bound) {
            printf("Error - Line %d - Identifier not declared\n", higher->term.type.tok.line_num);
            return -1;
        }
        if(!(bound->arr_info == PRIMITIVE && bound->type.prim_info == INTEGER)) {
            printf("Error - Line %d - Non integer bounds not allowed\n", higher->term.type.tok.line_num);
            return -1;
        }
        pairs[0].is_r2_static = 0;
        strcpy(pairs[0].r2.r_d, higher->term.type.tok.lexeme);
    }
    else {
        pairs[0].is_r2_static = 1;
        pairs[0].r2.r_s = atoi(higher->term.type.tok.lexeme);
    }
    if(pairs[0].is_r1_static && pairs[0].is_r2_static && (pairs[0].r1.r_s > pairs[0].r2.r_s)) {
        printf("Error - Line %d - Lower bound cannot exceed upper bound\n", higher->term.type.tok.line_num);
        return -1;
    }
    if(node->num_children == 3) return 0;
    return fill_ranges(node->children[3], pairs+1, table);
}

link * run_rect(parseTree * node, typeExpressionTable * table) {
    link * info = (link *) malloc(sizeof(link));
    info->arr_info = RECT_ARR;
    info->type.rect_arr_info.betype = INTEGER;
    int depth = get_depth(node->children[1]);
    info->type.rect_arr_info.num_dim = depth;
    info->type.rect_arr_info.dim_range = (Var_Pair *) malloc(depth * sizeof(Var_Pair));
    int success = fill_ranges(node->children[1], info->type.rect_arr_info.dim_range, table);
    if(success == -1) return NULL;
    return info;
}

link * get_type(parseTree * tree, int is_single, typeExpressionTable * table) {
    int dec = (is_single) ? 3 : 6;
    if(tree->children[dec]->children[0]->term.is_term == 0) return run_primitive(tree->children[dec]->children[0]);
    else if(tree->children[dec]->children[0]->term.type.tok.token == KEY_ARR) return run_rect(tree->children[dec], table);
    else return run_jagged(tree->children[dec], tree->children[dec+2]);
}

void traverseDeclares(parseTree * tree, typeExpressionTable * table) {
    if(!(tree->term.is_term == 0 && tree->term.type.nt == DECL_STMT)) {
        for(int i = 0; i < tree->num_children; i++) traverseDeclares(tree->children[i], table);
        return;
    }
    int single = (tree->children[0]->term.type.nt == SINGLEVAR_DEC);
    tree = tree->children[0];

    link * info = get_type(tree, single, table);
    if(single) {
        strcpy(info->id, tree->children[1]->term.type.tok.lexeme);
        put_link(table, info);
    }
    else add_info(tree->children[4], info, table);
}

link get_data_type_left(parseTree * tree, typeExpressionTable * table)
{
    if(tree->num_children==0)
    {
        char* lexeme = tree->term.type.tok.lexeme;
        int line_number = tree->term.type.tok.line_num;
        link* l = get_link(table,lexeme);
        if(!l)
        {
            printf("Variable %s Not Declared \n Line Number - %d \n Exiting!!", lexeme, line_number);
            exit(0);      // exiting program
        }
        tree->type_info = *l;
        return *l;
    }
    tree->type_info = get_data_type_left(tree->children[0], table); // assign same link to all nodes on left subtree (left-subtree of assgn is linear chain) 
    return tree->type_info;
}


int is_arithmetic_comaptible(link l1,link l2,char* err)
{
    if(l1.arr_info==l2.arr_info)
    {
        if(l1.arr_info==PRIMITIVE)
        {
            if(l1.type.prim_info!=l2.type.prim_info)
            {
                snprintf(err, 200, "PRIMITIVE TYPES %s and %s DON'T MATCH!!", l1.type.prim_info, l2.type.prim_info);
                return 0;
            }
            return 1;
        }
        if(l1.arr_info==RECT_ARR)
        {
            rect_arr_te r1 = l1.type.rect_arr_info;
            rect_arr_te r2 = l2.type.rect_arr_info;

            if(r1.betype!=INTEGER || r2.betype!=INTEGER)
            {
                snprintf(err, 200, "ONLY INTEGER ARRAYS ALLOWED!!");
                return 0;
            }
            if(r1.num_dim!=r2.num_dim)
            {
                snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
            }

            Var_Pair* v1 = r1.dim_range;
            Var_Pair* v2 = r2.dim_range;

            for(int i=0;i<r1.num_dim;i++)
            {
                if(v1[i].is_r1_static==0 || v1[i].is_r2_static==0 || v2[i].is_r1_static==0 || v2[i].is_r2_static==0) 
                {
                    snprintf(err, 200, "ARRAYS DIMENSIONS MUST BE FULLY SPECIFIED FOR ARITHMETIC!!");
                    return 0;
                }
                if(v1[i].r2.r_s - v1[i].r1.r_s != v2[i].r2.r_s - v2[i].r1.r_s)
                {
                    snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
                    return 0;
                }
            }
            return 1;
        }
        if(l1.arr_info==JAG_ARR)
        {
            jagged_arr_te r1 = l1.type.jagged_arr_info;
            jagged_arr_te r2 = l2.type.jagged_arr_info;

            if(r1.betype!=INTEGER || r2.betype!=INTEGER)
            {
                snprintf(err, 200, "ONLY INTEGER ARRAYS ALLOWED!!");
                return 0;
            }
            if(r1.num_dim!=r2.num_dim) // checks for 2D and 3D jagged array
            {
                snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
                return 0;
            }
            if(r1.range_R1.r2 - r1.range_R1.r1 != r2.range_R1.r2 - r2.range_R1.r1) // checks for same number of R1 dimensions
            {
                snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
                return 0;
            }

            Int_Pair* v1 = r1.range_R2;
            Int_Pair* v2 = r2.range_R2;

            for(int i=0;i<r1.num_dim;i++)
            {
                if(v1[i].r2 - v1[i].r1 != v2[i].r2 - v2[i].r1)
                {
                    snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
                    return 0;
                }
            }
            return 1;
        }
    }
    return 0;
}

link get_data_type_right_arithmetic(parseTree * tree, typeExpressionTable * table)
{
    if(tree->num_children==0)
    {
        char* lexeme = tree->term.type.tok.lexeme;
        int line_number = tree->term.type.tok.line_num;
        link* l = get_link(table,lexeme);
        if(!l)
        {
            printf("Variable %s Not Declared \n Line Number - %d \n Exiting!!", lexeme, line_number);
            exit(0);      // exiting program
        }
        tree->type_info = *l;
        return *l;
    }

    link d_left;
    link d_right;

    d_left = get_data_type_right_arithmetic(tree->children[0],table);

    if(tree->num_children==3)
    {
        char err_msg[200];
        int success;
        d_right = get_data_type_right_arithmetic(tree->children[2],table);
        success = is_arithmetic_comaptible(d_left,d_right,err_msg);

        if(!success)
        {
            printf("%s\n LINE-NUMBER %d\n", err_msg,tree->term.type.tok.line_num);
            exit(0);
        }

    }
    
    tree->type_info = d_left; // check this...assigns internal node of parse tree the same link as its left child( only same type expression is important here)

}


//TO-DO
link get_data_type_right_boolean(parseTree * tree, typeExpressionTable * table)
{
    
}

void traverseAssigns(parseTree * tree, typeExpressionTable * table) {
    if(!(tree->term.is_term == 0 && tree->term.type.nt == ASSGN_STMT)) {
        for(int i = 0; i < tree->num_children; i++) traverseAssigns(tree->children[i], table);
        return;
    }
    
    link type_left = get_data_type_left(tree->children[0],table);
    link type_right;

    int is_arithmetic = tree->children[2]->children[0]->term.type.nt == ARITHMETIC_EXP;

    if(is_arithmetic) type_right = get_data_type_right_arithmetic(tree->children[2],table); 
    else type_right = get_data_type_right_boolean(tree->children[2],table);
}


void traverseParseTree(parseTree *t, typeExpressionTable *Table) {
    traverseDeclares(t, Table);
    traverseAssigns(t, Table);
}