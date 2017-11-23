#include "generator.h"
#define DEBUG 0

#ifndef DEBUG
#define DEBUG 0
#endif

// INTERNAL //

typedef enum {
	f_global, f_local, f_temporary
} g_frame_level;

typedef enum {
    int2float, float2int, 
    float2r2eint, // preference sudé <<
    float2r2oint, // preference liché
    int2char, stri2int
} g_cast_type;

// Stack for conditions, loops
typedef struct gsti {
    s_stree node;
    unsigned id;
    int has_else;
    struct gsti *next;
} *gStackItem;

typedef struct gstc {
    gStackItem top;
} *gStack;

g_frame_level frame_level = f_local; // currentry used frame
unsigned sysv_next_id = 1; // next id of system / temporary variable
unsigned cfs_next_id = 1; // control flow statement next id (for labels and such)
s_stree fct = NULL; // currently generated function (no nesting allowed)
gStack stack = NULL; // conditions, loops stack (support for nesting ... no variable scope control)
int t_frame = 0;
unsigned g_errno = 0;

FILE *out;
unsigned level = 0;

void g_printlvl();

void printHeader();
void g_createTF();

char *stringify(char *);

/**
 * Generates expressions and temporary variables.
 * @param s_stree expression / const / variable node
 * @param int * is / isn't system generated variable
 * @param g_frame_level * frame in which variable is generated
 * @return s_stree variable with the result
 */
s_stree g_expression(s_stree, int *, g_frame_level *);

/**
 * Creates new frame.
 */
void g_createTF();

/**
 * Generates variable definition. (DEFVAR)
 * @param s_stree variable node
 */ 
void g_var(s_stree, int);

/**
 * Casts variable / const to given type.
 * If neccessary, temporary variable
 * is generated.
 * @param g_cast_type
 * @param s_stree const / variable node
 * @param g_frame_level  frame of casted value
 * @param int system of casted value
 * @param g_frame_level * frame in which variable is generated
 * @param int * is / isn't system generated variable
 */ 
s_stree g_cast(g_cast_type, s_stree, g_frame_level, int, g_frame_level *, int *);
s_stree g_cast_full(g_cast_type, s_stree, g_frame_level, int, s_stree, g_frame_level, int);
void g_castif(g_cast_type, s_stree, int);
void g_pop(s_stree, int);
void g_push(s_stree, int);
void g_move(s_stree, int, s_stree, int);
void g_move_frames(s_stree, g_frame_level, int, s_stree, g_frame_level, int);
void g_func_header(s_stree);
void g_func_params(s_stree);
void g_return(s_stree);
void g_func_end();
void g_call(s_stree);
void g_call_params(s_stree);
void g_cond(s_stree); // JUMPIFNEQ
void g_cond_else(s_stree);
void g_cond_end(s_stree);
void g_loop(s_stree);
void g_loop_end(s_stree);
void g_value(s_stree, int);
void g_print(s_stree, int);
void g_read(s_stree, int);


gStack gs_init();
gStackItem gs_push(gStack, s_stree, unsigned);
gStackItem gs_top(gStack);
void gs_pop(gStack);
void gs_destroy(gStack);

char *processTree(s_stree tree)
{
    if (tree == NULL)
        return "";

    level++;
    if (DEBUG) {
        g_printlvl();
        fprintf(out, "Node => %d | %d \n", tree->ntype, tree->dtype);
    }

    switch (tree->ntype) {
        case n_const: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Constant node \n");
            }
            //level--;
            //return g_constant(tree);
            break;
        }

        case n_var: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Var node \n");
            }
            g_var(tree, 0);
            break;
        }

        case n_expr: {
            if (DEBUG) {                
                g_printlvl();
                fprintf(out, "Expression node \n");
            }
            g_expression(tree, NULL, NULL);
            break;
        }

        case n_func: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Function node \n");
            }
            g_func_header(tree);
            break;
        }

        case n_return: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Return node \n");
            }
            g_return(tree);
            break;
        }

        case n_endfunc: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Function end node \n");
            }
            g_func_end(fct);
            break;
        }

        case n_call: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Call node \n");
            }
            g_call(tree);
            break;
        }

        case n_param: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Param node \n");
            }
            break;
        }

        case n_if: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Condition node \n");
            }

            g_cond(tree);
            break;
        }

        case n_else: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Else condition node \n");
            }

            g_cond_else(tree);
            break;
        }

        case n_endif: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "Condition end node \n");
            }

            g_cond_end(tree);
            break;
        }
        
        case n_while: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "White node \n");
            }

            g_loop(tree);
            break;
        }
        
        case n_endwhile: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "While end node \n");
            }

            g_loop_end(tree);
            break;
        }        
        
        case n_print: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "print node \n");
            }
            g_print(tree, 0);
            break;
        }        
        
        case n_read: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "print node \n");
            }
            g_read(tree, 0);
            break;
        }
    };

    level--;

    return "";
}

void printHeader()
{
    fprintf(out, ".IFJcode17\n");
    fprintf(out, "CREATEFRAME\n");
    fprintf(out, "PUSHFRAME\n");
}

void g_createTF()
{
    t_frame = 1;
    fprintf(out, "CREATEFRAME\n");
}

char *stringify(char *str)
{
    unsigned i = 0;
    char *rstr = (char *) malloc(sizeof(char) * i);
    //*rstr = '\0';

    while (str != NULL && *str != '\0') {
        if (*str  < 33 || *str == 35 || *str == 92) {
            i += 4;
            rstr = (char *) realloc(rstr, sizeof(char) * i);
            rstr[i - 4] = '\\';
            rstr[i - 3] = '0';


            if (*str < 10) {
                rstr[i - 2] = '0';
                rstr[i - 1] = *str + '0';
            } else {
                rstr[i - 2] = (int) *str / 10 + '0';
                rstr[i - 1] = (int) *str % 10 + '0';
            }
        } else {
            i++;
            rstr = (char *) realloc(rstr, sizeof(char) * i);
            rstr[i - 1] = *str;
        }

        str++;
    }

    i++;
    rstr = (char *) realloc(rstr, sizeof(char) * i);
    rstr[i - 1] = '\0';

    return rstr;
}

s_stree g_expression(s_stree tree, int *s, g_frame_level *src_frame) 
{
    s_stree target = NULL;
    if (s != NULL)
        *s = 0;

    if (strcmp(tree->value.v_string, "=") == 0) { // přiřazení
        target = tree->lptr;
        if (tree->rptr->ntype == n_const || tree->rptr->ntype == n_var) {
            g_move(target, 0, tree->rptr, 0);
        } else if (tree->rptr->ntype == n_expr) {
            int s = 0;
            g_frame_level l = frame_level;
            s_stree src = g_expression(tree->rptr, &s, &l);
            g_move_frames(target, frame_level, 0, src, l, s);
        } else if (tree->rptr->ntype == n_call) {
            g_call(tree->rptr);
            g_pop(target, 0);
        }
    } else { // vyhodnocení
        s_stree src1 = tree->lptr; int s1 = 0;
        s_stree src2 = tree->rptr; int s2 = 0;
        g_frame_level l1 = frame_level;
        g_frame_level l2 = frame_level;
        g_frame_level bl = frame_level;

        if (src_frame == NULL || t_frame == 0)
            g_createTF();

        if (src1->ntype == n_expr) {
            src1 = g_expression(src1, &s1, &l1);
        } else if (src1->ntype == n_call) {
            g_call(src1);
            char vname[10] = "tmp_";    
            sprintf(vname,"%s%d", vname, sysv_next_id++); 
            src1 = STcreateVar(vname, d_int); 
            s1 = 1;
            l1 = f_local;
            //l1 = f_temporary;
            frame_level = l1;
            g_var(src1, s1);
            g_pop(src1, s1);
            frame_level = bl;
        }

        if (src2->ntype == n_expr) {
            src2 = g_expression(src2, &s2, &l2);
        } else if (src2->ntype == n_call) {
            g_call(src2);
            char vname[10] = "tmp_";    
            sprintf(vname,"%s%d", vname, sysv_next_id++);  
            src2 = STcreateVar(vname, d_int);
            s2 = 1;
            l2 = f_local;
            //l2 = f_temporary;
            frame_level = l2;
            g_var(src2, s2);
            g_pop(src2, s2);
            frame_level = bl;
        }

        *src_frame = f_local;
        //*src_frame = f_temporary;
        frame_level = *src_frame;
        char vname[10] = "tmp_";    
        sprintf(vname,"%s%d", vname, sysv_next_id++);       
        target = STcreateVar(vname, d_int);
        *s = 1;
        g_var(target, *s);
        frame_level = bl;

        if (strcmp(tree->value.v_string, "==") == 0) {
            fprintf(out, "EQ "); target->dtype = d_bool;
        } else if (strcmp(tree->value.v_string, ">") == 0) {
            fprintf(out, "GT "); target->dtype = d_bool;
        } else if (strcmp(tree->value.v_string, "<") == 0) {
            fprintf(out, "LT "); target->dtype = d_bool;
        } else if (strcmp(tree->value.v_string, "AND") == 0 || strcmp(tree->value.v_string, "&&") == 0) {
            fprintf(out, "AND "); target->dtype = d_bool;
        } else if (strcmp(tree->value.v_string, "OR") == 0 || strcmp(tree->value.v_string, "||") == 0) {
            fprintf(out, "OR "); target->dtype = d_bool;
        } else if (strcmp(tree->value.v_string, "+") == 0) {
            if (src1->dtype == d_string && src2->dtype == d_string) {
                fprintf(out, "CONCAT "); target->dtype = d_string;
            } else {
                if (src1->dtype == d_double && src2->dtype == d_int) {src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);}
                else if (src1->dtype == d_int && src2->dtype == d_double) {src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);}
                fprintf(out, "ADD "); target->dtype = src1->dtype;
            }
        } else if (strcmp(tree->value.v_string, "-") == 0) {
            if (src1->dtype == d_double && src2->dtype == d_int) src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);
            else if (src1->dtype == d_int && src2->dtype == d_double) src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);
            fprintf(out, "SUB "); target->dtype = src1->dtype;
        } else if (strcmp(tree->value.v_string, "*") == 0) {            
            if (src1->dtype == d_double && src2->dtype == d_int) src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);
            else if (src1->dtype == d_int && src2->dtype == d_double) src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);
            fprintf(out, "MUL "); target->dtype = src1->dtype;
        } else if (strcmp(tree->value.v_string, "/") == 0) {
            if (src1->dtype == d_double && src2->dtype == d_int) src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);
            else if (src1->dtype == d_int && src2->dtype == d_double) src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);
            else { src1 = g_cast(int2float, src1, l1, s1, &l1, &s1); src2 = g_cast(int2float, src2, l2, s2, &l2, &s2); }
            fprintf(out, "DIV "); target->dtype = d_double;
        } else if (strcmp(tree->value.v_string, "\\") == 0) {

            // Pokud je některý double, zaokrouhli na int
            if (src1->ntype == n_var) {frame_level = l1; g_castif(float2r2eint, src1, s1);} else if (src1->dtype == d_double) {src1 = g_cast(float2r2eint, src1, l1, s1, &l1, &s1);} 
            if (src2->ntype == n_var) {frame_level = l2; g_castif(float2r2eint, src2, s2);} else if (src2->dtype == d_double) {src2 = g_cast(float2r2eint, src2, l2, s2, &l2, &s2);} 

            // Pro dělení, převeď na float
            if (src1->ntype == n_var) {frame_level = l1; g_castif(int2float, src1, s1);} else if (src1->dtype == d_int) {src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);} 
            if (src2->ntype == n_var) {frame_level = l2; g_castif(int2float, src2, s2);} else if (src2->dtype == d_int) {src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);} 

            fprintf(out, "DIV ");
            target->dtype = d_double;
            frame_level = *src_frame;
            g_value(target, 1); fprintf(out, " ");
            frame_level = l1;
            g_value(src1, s1); fprintf(out, " ");
            frame_level = l2;
            g_value(src2, s2); fprintf(out, "\n");
            frame_level = *src_frame;
            target = g_cast(float2int, target, frame_level, 1, NULL, NULL);
            frame_level = bl;

            return target;
        }

        frame_level = *src_frame;
        g_value(target, 1); fprintf(out, " ");
        frame_level = l1;
        g_value(src1, s1); fprintf(out, " ");
        frame_level = l2;
        g_value(src2, s2); fprintf(out, "\n");
        frame_level = bl;
    }

    return target;
}

void g_move(s_stree dest, int sd, s_stree src, int sr)
{
    g_move_frames(dest, frame_level, sd, src, frame_level, sr);
}

void g_move_frames(s_stree dest, g_frame_level dl, int sd, s_stree src, g_frame_level sl, int sr)
{
    g_frame_level back = frame_level;
    fprintf(out, "MOVE ");
    frame_level = dl;
    g_value(dest, sd);
    
    fprintf(out, " ");

    frame_level = sl;
    g_value(src, sr);
    fprintf(out, "\n");
    frame_level = back;
    dest->dtype = src->dtype;
}

void g_var(s_stree tree, int system)
{
    fprintf(out, "DEFVAR ");
    g_value(tree, system);
    fprintf(out, "\n");
}

s_stree g_cast(g_cast_type ct, s_stree dst, g_frame_level dst_l, int dst_s, g_frame_level *t_l, int *t_s)
{
    s_stree target = dst;
    g_frame_level target_l = dst_l;
    int target_s = dst_s;

    if (target->ntype == n_const) {
        char vname[10] = "tmp_";    
        sprintf(vname,"%s%d", vname, sysv_next_id++);       
        target = STcreateVar(vname, d_int);
        target_l = frame_level;
        *t_l = f_local;
        //*t_l = f_temporary;
        frame_level = *t_l;
        *t_s = 1;
        g_var(target, *t_s);
        frame_level = target_l;
        target_l = *t_l;
        target_s = *t_s;
    }

    return g_cast_full(ct, target, target_l, target_s, dst, dst_l, dst_s);
}

s_stree g_cast_full(g_cast_type ct, s_stree dst, g_frame_level dst_l, int dst_s, s_stree src, g_frame_level src_l, int src_s)
{
    g_frame_level bl = frame_level;
    switch (ct) {
        case int2float: {
            fprintf(out, "INT2FLOAT ");
            dst->dtype = d_double;
            break;
        }

        case float2int: {            
            fprintf(out, "FLOAT2INT ");
            dst->dtype = d_int;
            break;
        }

        case float2r2eint: {            
            fprintf(out, "FLOAT2R2EINT ");
            dst->dtype = d_int;
            break;
        }

        case float2r2oint: {            
            fprintf(out, "FLOAT2R2OINT ");
            dst->dtype = d_int;
            break;
        }

        case int2char: {            
            fprintf(out, "INT2CHAR ");
            dst->dtype = d_string;
            break;
        }

        default: {
            break;
        }
    }

    frame_level = dst_l;
    g_value(dst, dst_s);
    fprintf(out, " ");
    frame_level = src_l;
    g_value(src, src_s);
    fprintf(out, "\n");
    frame_level = bl;

    return dst;
}

void g_castif(g_cast_type type, s_stree tree, int system)
{
    g_frame_level bl = frame_level;
    frame_level = f_local;  // při změně změnit LF@%s => TF@%s
    //frame_level = f_temporary;
    char vname[10] = "tmp_";    
    sprintf(vname,"%s%d", vname, sysv_next_id++);       
    s_stree tmp = STcreateVar(vname, d_string);
    g_var(tmp, 1);
    frame_level = bl;
    fprintf(out, "TYPE LF@%s ", tmp->value.v_string);
    g_value(tree, system);
    fprintf(out, "\n");
    fprintf(out, "JUMPIFNEQ cast_if_%d LF@%s string@", cfs_next_id++, tmp->value.v_string);

    if (type == int2char || type == int2float) {
        fprintf(out, "int");
    } else if (type == stri2int) {
        fprintf(out, "string");
    } else if (type == float2int || type == float2r2eint || type == float2r2oint) {
        fprintf(out, "float");
    }

    fprintf(out, "\n");
    g_cast(type, tree, frame_level, system, NULL, NULL);
    fprintf(out, "LABEL cast_if_%d\n", cfs_next_id - 1);
}

void g_pop(s_stree tree, int s)
{    
    fprintf(out, "POPS ");
    g_value(tree, s);
    fprintf(out, "\n");
}

void g_push(s_stree tree, int s)
{    
    fprintf(out, "PUSHS ");
    g_value(tree, s);
    fprintf(out, "\n");
}

void g_func_header(s_stree tree) 
{
    fct = tree;
    fprintf(out, "JUMP fcaf_%s\n\nLABEL fc_%s\n", tree->value.v_string, tree->value.v_string);
    fprintf(out, "CREATEFRAME\n");
    fprintf(out, "PUSHFRAME\n");

    if (tree->rptr != NULL)
        g_func_params(tree->rptr);
}

void g_func_params(s_stree tree)
{
    while(tree != NULL) {
        g_var(tree->lptr, 0);
        g_pop(tree->lptr, 0);
        tree = tree->rptr;
    }
}

void g_return(s_stree tree)
{
    s_stree returning_node = tree->rptr;
    int ret_s = 0;
    g_frame_level ret_l = frame_level;
    g_frame_level bl = frame_level;

    if (returning_node->ntype == n_expr) {
        g_createTF();
        returning_node = g_expression(returning_node, &ret_s, &ret_l);
    } else if (returning_node->ntype == n_call) {
        g_call(returning_node);
        char vname[10] = "tmp_";    
        sprintf(vname,"%s%d", vname, sysv_next_id++);       
        returning_node = STcreateVar(vname, d_int);
        ret_s = 1;
        ret_l = f_local;
        //ret_l = f_temporary;
        frame_level = ret_l;
        g_var(returning_node, ret_s);
        g_pop(returning_node, ret_s);
    }

    frame_level = ret_l;
    if (returning_node->dtype != fct->dtype) { // je potřeba převést na daný typ int -> double, double -> int
        if (fct->dtype == d_double) {
            g_castif(int2float, returning_node, ret_s);
            //returning_node = g_cast(int2float, returning_node, ret_l, ret_s, &ret_l, &ret_s);
        } else if (fct->dtype == d_int) {
            g_castif(float2r2eint, returning_node, ret_s);
            //returning_node = g_cast(float2r2eint, returning_node, ret_l, ret_s, &ret_l, &ret_s);
        }
    }

    g_push(returning_node, ret_s);
    frame_level = bl;
    fprintf(out, "JUMP fce_%s\n", fct->value.v_string);
}

void g_func_end(s_stree tree)
{  
    (void) tree;

    if (tree != NULL) {
        fprintf(out, "LABEL fce_%s\n", tree->value.v_string);
    }

    fprintf(out, "POPFRAME\n");
    fprintf(out, "RETURN\n");
    
    if (tree != NULL) {
        fprintf(out, "LABEL fcaf_%s\n\n", tree->value.v_string);
    }
}

void g_call(s_stree tree)
{
    if (tree->rptr != NULL)
        g_call_params(tree->rptr);

    fprintf(out, "CALL fc_%s\n", tree->value.v_string);
}

void g_call_params(s_stree tree)
{
    if (tree == NULL) {
        return;
    }

    g_call_params(tree->rptr);
    g_push(tree->lptr, 0);
}

void g_cond(s_stree tree)
{
    if (gs_push(stack, tree, cfs_next_id++) == NULL) {
        g_errno = G_ERROR_MEMORY | G_ERROR_STACK;
        return;
    }

    s_stree r_var = tree->rptr;
    int s = 0;
    g_frame_level l = frame_level;
    g_frame_level bl = frame_level;
    if (tree->rptr->ntype == n_expr) {
        g_createTF();
        r_var = g_expression(r_var, &s, &l);
    }

    fprintf(out, "JUMPIFNEQ conde_%d ", cfs_next_id - 1);
    frame_level = l;
    g_value(r_var, s);
    fprintf(out, " bool@true\n");
    frame_level = bl;
}

void g_cond_else(s_stree tree)
{
    (void) tree;

    gStackItem item = gs_top(stack);
    item->has_else = 1;

    fprintf(out, "JUMP conde_else_%d\n", item->id);
    fprintf(out, "LABEL conde_%d\n", item->id);
}

void g_cond_end(s_stree tree)
{
    (void) tree;

    gStackItem item = gs_top(stack);
    if (item == NULL) {
        g_errno = G_ERROR_STACK;
        return;
    }

    if (item->has_else) {
        fprintf(out, "LABEL conde_else_%d\n", item->id);
    } else {
        fprintf(out, "LABEL conde_%d\n", item->id);
    }

    gs_pop(stack);
}

void g_loop(s_stree tree)
{
    if (gs_push(stack, tree, cfs_next_id++) == NULL) {
        g_errno = G_ERROR_MEMORY | G_ERROR_STACK;
        return;
    }

    fprintf(out, "LABEL loopa_%d\n", cfs_next_id - 1);
    
    s_stree r_var = tree->rptr;
    int s = 0;
    g_frame_level l = frame_level;
    g_frame_level bl = frame_level;
    if (tree->rptr->ntype == n_expr) {
       g_createTF();
       r_var = g_expression(r_var, &s, &l);
    }

    frame_level = l;
    fprintf(out, "JUMPIFNEQ loope_%d ", cfs_next_id - 1);
    g_value(r_var, s);
    fprintf(out, " bool@true\n");
    frame_level = bl;
}

void g_loop_end(s_stree tree)
{
    (void) tree;

    gStackItem item = gs_top(stack);
    if (item == NULL) {
        g_errno = G_ERROR_STACK;
        return;
    }

    fprintf(out, "JUMP loopa_%d\n", item->id);
    fprintf(out, "LABEL loope_%d\n", item->id);
    gs_pop(stack);
}

void g_value(s_stree tree, int system)
{
    switch (tree->ntype) {
        case n_const: {
            if (tree->dtype == d_int) {
                fprintf(out, "int@%d", tree->value.v_int);
            } else if (tree->dtype == d_double) {
                fprintf(out, "float@%g", tree->value.v_double);
            } else {
                char *so = stringify(tree->value.v_string);
                fprintf(out, "string@%s", so);
                free(so);
            }
            break;
        }
    
        case n_var: {                
            (frame_level == f_global) ? fprintf(out, "GF@") : (frame_level == f_local ? fprintf(out, "LF@") : fprintf(out, "TF@"));
            if (!system) fprintf(out, "v_");
            fprintf(out, "%s", tree->value.v_string);
            break;
        }

        default: {
            break;
        }
    }
}

void g_print(s_stree tree, int system)
{
    s_stree r_var = tree->rptr;
    g_frame_level l = frame_level;
    g_frame_level bl = frame_level;
    if (r_var->ntype == n_expr) {
        r_var = g_expression(tree->rptr, &system, &l);
    } else if (r_var->ntype == n_call) { // check if it's returning function?
        g_call(r_var);
        char vname[10] = "tmp_";    
        sprintf(vname,"%s%d", vname, sysv_next_id++);       
        r_var = STcreateVar(vname, d_int);
        system = 1;
        l = f_local;
        //l = f_temporary;
        frame_level = l;
        g_var(r_var, system);
        g_pop(r_var, system);
    }

    fprintf(out, "WRITE ");
    frame_level = l;
    g_value(r_var, system);
    frame_level = bl;
    fprintf(out, "\n");
}

void g_read(s_stree tree, int system) 
{
    fprintf(out, "WRITE string@?\\032\n");
    fprintf(out, "READ ");
    g_value(tree->rptr, system);
    
    switch (tree->rptr->dtype) {
        case d_bool: {
            fprintf(out, " bool");
            break;
        }

        case d_int: {
            fprintf(out, " int");
            break;
        }

        case d_string: {
            fprintf(out, " string");
            break;
        }

        case d_double: {
            fprintf(out, " float");
            break;
        }

        default: {
            break;
        }
    }

    fprintf(out, "\n");
}

void g_printlvl()
{
    for (unsigned i = 0; i < level; i++)
        fputc('\t', out);
}

gStack gs_init()
{
    gStack stack = (gStack) malloc(sizeof(struct gstc));
    if (stack == NULL)
        return NULL;

    stack->top = NULL;
    return stack;
}

gStackItem gs_push(gStack stack, s_stree node, unsigned id)
{
    gStackItem item = (gStackItem) malloc(sizeof(struct gsti));
    if (item == NULL)
        return NULL;

    item->next = stack->top;
    item->id = id;
    item->node = node;
    item->has_else = 0;

    stack->top = item;

    return item;
}

gStackItem gs_top(gStack stack)
{
    return stack->top;
}

void gs_pop(gStack stack)
{
    gStackItem item = stack->top;
    if (item != NULL) {
        stack->top = item->next;
        free(item);
    }
}

void gs_destroy(gStack stack)
{
    for(gStackItem item = stack->top; item != NULL;) {
        gStackItem fItem = item;
        item = item->next;
        free(fItem);
    }

    free(stack);
}

// EXTARNAL //

int generate(s_list list, FILE *f)
{
    out = f;
    stack = gs_init();
    printHeader();

    for (int i = 0; i < list->size && !g_errno; i++) {
        processTree(Lget(&list, i));
    }

    if (stack->top != NULL)
        g_errno = G_ERROR_STACK | G_ERROR_NOT_EMPTY;
    
    fprintf(out, "CLEARS\n");
    
    gs_destroy(stack);

    return g_errno;
}