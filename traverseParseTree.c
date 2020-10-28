#include "parser.h"
#include <assert.h>

link * copy_link(link * source) {
    link * new = (link *) malloc(sizeof(link));
    new->arr_info = source->arr_info;
    new->arr_storage = source->arr_storage;
    new->type = source->type;
   return new;
}

void add_info(parseTree * node, link * info, typeExpressionTable * table) {
    strcpy(info->id, node->children[0]->term.type.tok.lexeme);
    printf("single %d prj %d%d%d sdn %d%d%d id %s ", 0, info->arr_info==PRIMITIVE, info->arr_info==RECT_ARR, info->arr_info==JAG_ARR, info->arr_storage==STATIC, info->arr_storage==DYNAMIC, info->arr_storage==NONE, info->id);
    if(info->arr_info==PRIMITIVE) printf("irb %d%d%d\n", info->type.prim_info == INTEGER, info->type.prim_info == REAL, info->type.prim_info == BOOLEAN);
    if(info->arr_info==RECT_ARR) printf("irb %d%d%d\n", info->type.rect_arr_info.betype == INTEGER, info->type.rect_arr_info.betype == REAL, info->type.rect_arr_info.betype == BOOLEAN);
    if(info->arr_info==JAG_ARR) printf("irb %d%d%d\n", info->type.jagged_arr_info.betype == INTEGER, info->type.jagged_arr_info.betype == REAL, info->type.jagged_arr_info.betype == BOOLEAN);
    put_link(table, info);
    if(node->num_children > 1) add_info(node->children[1], copy_link(info), table);
}

int get_length_idx(parseTree * idx) {
    if(idx->num_children == 1 && idx->children[0]->term.type.nt == EPSILON) return 0;
    return 1 + get_length_idx(idx->children[1]);
}

link * run_primitive(parseTree * node) {
    link * info = (link *) malloc(sizeof(link));
    info->arr_info = PRIMITIVE;
    info->arr_storage = NONE;
    // printf("Is Bool %d\n", node->children[0]->term.type.tok.token == KEY_BOOL);
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
        int line_num;
        for(int i = 0; i < range_1; i++) {
            info->type.jagged_arr_info.range_R2[i].num_dim = 1;
            info->type.jagged_arr_info.range_R2[i].dims = (int *) malloc(sizeof(int));
        }
        int count = 0;
        while(count < range_1) {
            line_num = init->children[0]->children[0]->term.type.tok.line_num;
            info->type.jagged_arr_info.range_R2[count++].dims[0] = atoi(init->children[0]->children[6]->term.type.tok.lexeme);
            if(init->num_children == 1 || init->children[1]->num_children > 1 && count == range_1-1) {
                char buf[25];
                get_str(init->children[0]->term.type, buf, init->children[0]->term.is_term);
                printf("%s\n", buf);
                printf("Error - Line %d - R1 Size mismatch %d\n", line_num+1, init->children[0]->term.type.tok.token==INT);
                return NULL;
            }
            if(count == range_1 && init->num_children > 1) {
                printf("Huh?\n");
            }
            init = init->children[1];
        }
        printf("%d %d\n", count, init->num_children);
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
            if(range->num_children < 3 && (j < num_dim - 1) || range->num_children == 3 && (j == num_dim - 1)) {
                printf("Error - Line %d - R2 Size mismatch\n", range->children[0]->children[0]->term.type.tok.line_num);
                return NULL;
            }
            else if(j < num_dim-1) range = range->children[2];
        }
        if(init->num_children < 2 && (i < range_1 - 1) || init->num_children == 2 && (i == range_1 - 1)) {
            printf("Error - Line %d - R1 Size mismatch\n", init->children[0]->children[6]->term.type.tok.line_num);
            return NULL;
        }
        if(i < range_1 - 1) init = init->children[1];
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
    printf("Dims %d\n", depth);
    for(int i = 0; i < depth; i++) printf("Range %d to %d\n", info->type.rect_arr_info.dim_range[i].r1.r_s, info->type.rect_arr_info.dim_range[i].r2.r_s);
    for(int i = 0; i < depth; i++) printf("Range %s to %s\n", info->type.rect_arr_info.dim_range[i].r1.r_d, info->type.rect_arr_info.dim_range[i].r2.r_d);
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
    if(info == NULL) return;
    if(single) {
        printf("single %d prj %d%d%d sdn %d%d%d id %s ", 1, info->arr_info==PRIMITIVE, info->arr_info==RECT_ARR, info->arr_info==JAG_ARR, info->arr_storage==STATIC, info->arr_storage==DYNAMIC, info->arr_storage==NONE, tree->children[1]->term.type.tok.lexeme);
        if(info->arr_info==PRIMITIVE) printf("irb %d%d%d\n", info->type.prim_info == INTEGER, info->type.prim_info == REAL, info->type.prim_info == BOOLEAN);
        if(info->arr_info==RECT_ARR) printf("irb %d%d%d\n", info->type.rect_arr_info.betype == INTEGER, info->type.rect_arr_info.betype == REAL, info->type.rect_arr_info.betype == BOOLEAN);
        if(info->arr_info==JAG_ARR) printf("irb %d%d%d\n", info->type.jagged_arr_info.betype == INTEGER, info->type.jagged_arr_info.betype == REAL, info->type.jagged_arr_info.betype == BOOLEAN);
        strcpy(info->id, tree->children[1]->term.type.tok.lexeme);
        put_link(table, info);
    }
    else add_info(tree->children[4], info, table);
}


/****      ASSIGNMENT TRAVERSAL   ********/

int is_op_compatible(link l1, link l2, Token tok, char* err)
{
    if(l1.arr_info==l2.arr_info)
    {
        if(l1.arr_info==PRIMITIVE)
        {
            if(l1.type.prim_info!=l2.type.prim_info)
            {
                snprintf(err, 200, "PRIMITIVE TYPES DON'T MATCH!!");
                return 0;
            }

            if(l1.type.prim_info==INTEGER || l1.type.prim_info==REAL)
            {
                if(tok == OP_OR || tok == OP_AND)
                {
                    snprintf(err, 200, "BOOLEAN OPERATIONS CANT BE PERFORMED ON INTEGER OR REAL VARIABLES!!");
                    return 0;
                }
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

            if(tok==OP_AND || tok==OP_OR || tok==OP_SLASH)
            {
                snprintf(err, 200, "INVALID ARRAY OPERATION");
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

            if(tok==OP_AND || tok==OP_OR || tok==OP_SLASH)
            {
                snprintf(err, 200, "INVALID ARRAY OPERATION");
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

            rng_R2* v1 = r1.range_R2;
            rng_R2* v2 = r2.range_R2;

            for(int i=0;i<r1.range_R1.r2-r1.range_R1.r1 + 1;i++)
            {
                if(v1[i].num_dim != v2[i].num_dim) 
                {
                    snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
                    return 0;
                }
                for(int j=0;j<v1[i].num_dim;j++)
                {
                    if(v1[i].dims[j] != v2[i].dims[j])
                    {
                        snprintf(err, 200, "ARRAYS MUST HAVE SAME DIMENSION FOR ARITHMETIC!!");
                        return 0;
                    }
                }
            }
            return 1;
        }
    }

    snprintf(err, 200, "TYPES DO NO MATCH!!");
    return 0;
}

link get_data_type_of_id(parseTree * tree, typeExpressionTable * table, int* line_number, int* success, char* err_msg) // tree points to parse tree node with token '<ID>'
{
    char* lexeme = tree->term.type.tok.lexeme;
    *line_number = tree->term.type.tok.line_num;

    link* l = get_link(table,lexeme);
    if(!l)
    {
        snprintf(err_msg, 200, "Variable %s Not Declared", lexeme);
        *success=0;
        return (link){};     
    }
    tree->type_info = *l;
    return *l;
}

link get_data_type_var(parseTree * tree, typeExpressionTable * table, int* line_number, int* success, char* err_msg)
{

    link l1 = get_data_type_of_id(tree->children[0],table, line_number,success,err_msg);
    if(!*success) return (link){};
  
    if(tree->num_children==2 && l1.arr_info==PRIMITIVE)
    {
        //printf("var is %s \n", tree->children[0]->term.type.tok.lexeme);
        snprintf(err_msg, 200, "CANNOT INDEX A PRIMITIVE TYPE!!");
        *success=0;
        return (link){};
    }

    if(tree->num_children==2)
    {
        parseTree *r = tree->children[1];   // 'r' is pointing to parse tree node whose token is '<array_idx>'
        int curr_idx=0;
        int size=1;
        int* dims = malloc(1 * sizeof(int));
        for(int i=1;i<r->num_children-1;i++) // skip '[' and ']' of array index
        {
            parseTree* tmp = r->children[i]->children[0]->children[0];   // in our grammar '<array_sel>' has only one child '<index>' which can either be '<var>' or '<int'>
            if(tmp->term.type.tok.token==ID)
            {
                snprintf(err_msg, 200, "ARRAY DIMENSIONS MUST BE FULLY SPECIFIED FOR ARITHMETIC");
                *success=0;
                return (link){};
            }

            assert(tmp->term.type.tok.token == INT);

            if(curr_idx+1>size) dims = realloc(dims, size*=2);
            
            dims[curr_idx++] = atoi(tmp->term.type.tok.lexeme);

        }
        dims = realloc(dims,curr_idx);

        if(l1.arr_info==RECT_ARR)
        {
            Var_Pair* found_dims = l1.type.rect_arr_info.dim_range;    // this and next 2 values obtained from type expression of variable
            int found_num_dims = l1.type.rect_arr_info.num_dim;

            if(curr_idx != found_num_dims)
            {
                snprintf(err_msg, 200, "NUMBER OF ARRAY DIMENSIONS DO NOT MATCH !!\n Line-Number - %d \n", *line_number);
                *success=0;
                return (link){};
            }

            for(int i=0;i<curr_idx;i++)
            {
                if(found_dims[i].is_r1_static==0 || found_dims[i].is_r2_static==0) 
                {
                    snprintf(err_msg, 200, "ARRAY VARIABLE DIMENSION DECLARED DYNAMIC SO UNABLE TO CHECK!!");
                    *success=0;
                    return (link){};
                }

                if(!(dims[i]>=found_dims[i].r1.r_s && dims[i] <= found_dims[i].r2.r_s))
                {
                    snprintf(err_msg, 200, "ARRAY BOUNDS OUT OF RANGE!!");
                    *success=0;
                    return (link){};
                }
            }

            l1.arr_info = PRIMITIVE;
            l1.type.prim_info = l1.type.rect_arr_info.betype;   // after indexing array element, it is a primitive integer

        }

        if(l1.arr_info==JAG_ARR)
        {
            Int_Pair found_dims_R1 = l1.type.jagged_arr_info.range_R1;    // this and next 3 values obtained from type expression of variable
            rng_R2* found_dims_R2 = l1.type.jagged_arr_info.range_R2; 
            int found_num_dims = l1.type.jagged_arr_info.num_dim;

            if(curr_idx != found_num_dims)
            {
                snprintf(err_msg, 200, "NUMBER OF ARRAY DIMENSIONS DO NOT MATCH !!");
                *success=0;
                return (link){};
            }

            if(!(dims[0]>=found_dims_R1.r1 || dims[0] <= found_dims_R1.r2)) 
            {
                snprintf(err_msg, 200, "ARRAY R1 DIMENSIONS OUT OF BOUND!!");
                *success=0;
                return (link){};
            }

            if(curr_idx==2)  // if jagged array has dimension 2
            {
                    int a = dims[1]>=found_dims_R2[0].dims[0] && dims[1] <= found_dims_R2[1].dims[0];
                    if (!a)
                    {
                        snprintf(err_msg, 200, "ARRAY R2 DIMENSIONS OUT OF BOUND!!");
                        *success=0;
                        return (link){};
                    }
            }
            

            if(curr_idx==3)  // if jagged array has dimension 3
            {
                int id1 = dims[0] - found_dims_R1.r1;
                rng_R2 req = found_dims_R2[id1];
                if(dims[1]>=req.num_dim || dims[1]<0)
                {
                    snprintf(err_msg,200, "ARRAY 2ND DIMENSION OUT OF BOUNDS!!");
                    *success=0;
                    return (link){};
                }
                if(dims[2]>=req.dims[dims[1]] || dims[2]<0)
                {
                    snprintf(err_msg,200, "ARRAY 3RD DIMENSION OUT OF BOUNDS!!");
                    *success=0;
                    return (link){};
                }
            }

            l1.arr_info = PRIMITIVE;
            l1.type.prim_info = l1.type.rect_arr_info.betype;   // after indexing array element, it is a primitive integer
        }
    }
    return l1;
}


link get_data_type_right(parseTree * tree, typeExpressionTable * table, int* success, char* err_msg) // gets data type to right of assignment statement
{
    if(!*success) return (link){}; 

    int tmp_line_no;

    if(tree->term.type.nt==VAR) return get_data_type_var(tree, table, &tmp_line_no, success, err_msg);

    if(tree->num_children==0) // needed for <INT> tokens
    {
        char* lexeme = tree->term.type.tok.lexeme;
        Token tk = tree->term.type.tok.token;
        if(tk==INT)
        {
            type_exp te = {INTEGER};
            link l;
            l.arr_info = PRIMITIVE;
            l.arr_storage = STATIC;
            l.type = te;
            tree->type_info = l;
            return l;
        }

        link* l = get_link(table,lexeme);
        if(!l)
        {
            snprintf(err_msg, 200, "Variable %s Not Declared \n Exiting!!", lexeme);
            *success=0;
            return (link){};    
        }
        tree->type_info = *l;
        return *l;
    }

    link d_left;
    link d_right;

    d_left = get_data_type_right(tree->children[0],table,success,err_msg);
    if(!*success) return (link){}; 

    if(tree->num_children==3)
    {
        d_right = get_data_type_right(tree->children[2],table,success,err_msg);
        if(!*success) return (link){}; 


        *success = is_op_compatible(d_left,d_right,tree->children[1]->term.type.tok.token,err_msg);
        if(!*success) return (link){}; 
        
        if(tree->children[1]->term.is_term==1 && tree->children[1]->term.type.tok.token==OP_SLASH)
        {
            d_left.type.prim_info=REAL;
        }
    }
    
    tree->type_info = d_left;
    return tree->type_info;
}

void traverseAssigns(parseTree * tree, typeExpressionTable * table) {
    if(!(tree->term.is_term == 0 && tree->term.type.nt == ASSGN_STMT)) {
        for(int i = 0; i < tree->num_children; i++) traverseAssigns(tree->children[i], table);
        return;
    }

    int line_number;
    int success=1;
    char err_msg[200];


    link type_left = get_data_type_var(tree->children[0], table, &line_number,&success,err_msg);

    if(!success)
    {
        printf("%s\n Line Number - %d\n", err_msg, line_number);
        return;
    }

    link type_right = get_data_type_right(tree->children[2],table,&success,err_msg); 

    if(!success)
    {
        printf("%s\n Line Number - %d\n", err_msg, line_number);
        return;
    }

    //printf("actual triple LN - %d\n", line_number);
    
    if(!is_op_compatible(type_left,type_right,ASSGN,err_msg))
    {
        printf("%s\n LINE-NUMBER %d \n", err_msg , line_number);
        return;
    }
}


void traverseParseTree(parseTree *t, typeExpressionTable *Table) {
    traverseDeclares(t, Table);
    traverseAssigns(t, Table);

    // link* l1 = get_link(Table, "xyz");
    // printf("type is %d \n", l1->arr_info);
}
