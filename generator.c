/**
 * Projekt - Tým 097, varianta I
 * Autor: Tom Barbořík (xbarbo06)
 * Generátor cílového kódu pro interpret
 */

#include "generator.h"
#define DEBUG 0

#ifndef DEBUG
#define DEBUG 0
#endif

#define S_EQ(d, w) strcmp((d), (w)) == 0

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
int loop_expr = 0;
unsigned g_errno = 0;

FILE *out;
unsigned level = 0;

void g_printlvl();

void printHeader();
void g_createTF();

unsigned char *stringify(char *);


extern s_btree t_symtable;
extern s_btree g_symtable;

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
void g_cast_self(g_cast_type, s_stree tree, g_frame_level, int);
s_stree g_cast(g_cast_type, s_stree, g_frame_level, int, g_frame_level *, int *);
s_stree g_cast_full(g_cast_type, s_stree, g_frame_level, int, s_stree, g_frame_level, int);
void g_castif(g_cast_type, s_stree, int);
s_stree g_castif_full(g_cast_type, s_stree, int, g_frame_level, s_stree, int, g_frame_level, int *, g_frame_level *);
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
void g_buildIn();
void g_buildInLength();
void g_buildInSubStr();
void g_buildInAsc();
void g_buildInChr();
s_stree g_tmp(g_frame_level, e_dstype);


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

        case n_i2f: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "i2f node \n");
            }
            g_cast_self(int2float, tree, f_local, 0);
            break;
        }

        case n_f2i: {
            if (DEBUG) {
                g_printlvl();
                fprintf(out, "f2i node \n");
            }
            g_cast_self(float2r2eint, tree, f_local, 0);
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

unsigned char *stringify(char *str)
{
    unsigned i = 0;
    unsigned char *rstr = (unsigned char *) malloc(sizeof(char) * i);
    //printf(">> %s\n <<", str);
    //*rstr = '\0';

    while (str != NULL && *str != '\0') {
        unsigned char c = (unsigned char) *str;
        
        if (c  < 33 || c == 35 || c == 92) {
            //printf(">> prevadim znak: %d - %c\n", (char) *str, *str);
            i += 4;
            rstr = (unsigned char *) realloc(rstr, sizeof(unsigned char) * i);
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
            rstr = (unsigned char *) realloc(rstr, sizeof(unsigned char) * i);
            rstr[i - 1] = *str;
        }

        str++;
    }

    i++;
    rstr = (unsigned char *) realloc(rstr, sizeof(char) * i);
    rstr[i - 1] = '\0';

    return rstr;
}

s_stree g_expression(s_stree tree, int *s, g_frame_level *src_frame) 
{
    s_stree target = NULL;
    if (s != NULL)
        *s = 0;

    if (strcmp(tree->value.v_string, "asg") == 0) { // přiřazení
        target = tree->lptr;
        if (tree->rptr->ntype == n_const || tree->rptr->ntype == n_var) {
            g_move(target, 0, tree->rptr, 0);
        } else if (tree->rptr->ntype == n_expr) {
            int s1 = 0;
            g_frame_level l = frame_level;
            s_stree src = g_expression(tree->rptr, &s1, &l);
            g_move_frames(target, frame_level, 0, src, l, s1);
        } else if (tree->rptr->ntype == n_call) {
            g_call(tree->rptr);
            g_pop(target, 0);
        }
    } else { // vyhodnocení
        s_stree src1 = tree->lptr; 
        int s1 = 0;
        s_stree src2 = tree->rptr; 
        int s2 = 0;
        g_frame_level l1 = frame_level;
        g_frame_level l2 = frame_level;
        g_frame_level bl = frame_level;
        char *oper = tree->value.v_string;


        if (src_frame == NULL || t_frame == 0)
            g_createTF();
        if (src1->ntype == n_expr) {
            src1 = g_expression(src1, &s1, &l1);
        } else if (src1->ntype == n_call) { //  nemělo by být součástí výrazů, pouze výrazu typu přiřazení
            g_call(src1);
            s1 = 1;
            l1 = (loop_expr == 1) ? f_temporary : f_local;
            src1 = g_tmp(l1, d_undef);   
            frame_level = l1;
            g_pop(src1, s1);
            frame_level = bl;
        }

        if (src2->ntype == n_expr) {
            src2 = g_expression(src2, &s2, &l2);
        } else if (src2->ntype == n_call) { //  nemělo by být součástí výrazů, pouze výrazu typu přiřazení          
            g_call(src2);
            s2 = 1;
            l2 = (loop_expr == 1) ? f_temporary : f_local;
            src2 = g_tmp(l2, d_undef);   
            frame_level = l2;
            g_pop(src1, s2);
            frame_level = bl;
        }

        *src_frame = (loop_expr == 1) ? f_temporary : f_local;
        *s = 1;
        target = g_tmp(*src_frame, tree->dtype);

        if ((S_EQ(oper, "+") && src1->dtype != d_string) || S_EQ(oper, "-") || S_EQ(oper, "*") || S_EQ(oper, "=") || S_EQ(oper, ">") || S_EQ(oper, "<") || S_EQ(oper, "<>") || S_EQ(oper, "<=") || S_EQ(oper, ">=")) {
            // konstanta + proměnná přetyp na vyšší typ
            // podle konstanty
            if (src1->ntype == n_const && src1->dtype == d_double && src2->ntype == n_var) {
                //printf("cast 1\n");
            } else if (src2->ntype == n_const && src2->dtype == d_double && src1->ntype == n_var) { 
                g_castif(int2float, src1, s1); 
            } else {
                g_frame_level c_tmp_l = (loop_expr == 1) ? f_temporary : f_local;
                s_stree c_tmp = g_tmp(c_tmp_l, d_double);
                g_move_frames(c_tmp, c_tmp_l, 1, src1, l1, s1);
                src1 = g_castif_full(int2float, c_tmp, 1, c_tmp_l, src2, s2, l2, &s1, &l1);
                s1 = 1;
                l1 = c_tmp_l;
                
                c_tmp = g_tmp(c_tmp_l, d_double);
                g_move_frames(c_tmp, c_tmp_l, 1, src2, l2, s2);
                src2 = g_castif_full(int2float, c_tmp, 1, c_tmp_l, src1, s1, l1, &s2, &l2);
                s2 = 1;
                l2 = c_tmp_l;
                //src2 = g_castif_full(int2float, src2, s2, l2, src1, s1, l1, &s2, &l2);
            }
        }

        if (S_EQ(oper, "=")) {
            fprintf(out, "EQ "); target->dtype = d_bool;
        } else if (S_EQ(oper, ">")) {
            fprintf(out, "GT "); target->dtype = d_bool;
        } else if (S_EQ(oper, "<")) {
            fprintf(out, "LT "); target->dtype = d_bool;
        } else if (S_EQ(oper, "<>")) {
            fprintf(out, "EQ "); target->dtype = d_bool;           
            frame_level = *src_frame;
            g_value(target, 1); fprintf(out, " ");
            frame_level = l1;
            g_value(src1, s1); fprintf(out, " ");
            frame_level = l2;
            g_value(src2, s2); fprintf(out, "\n");
            frame_level = bl;
            target->dtype = d_bool;

            fprintf(out, "NOT ");     
            frame_level = *src_frame;
            g_value(target, 1); fprintf(out, " ");     
            frame_level = *src_frame;
            g_value(target, 1); fprintf(out, "\n");
            frame_level = bl;
            target->dtype = d_bool;

            return target;
        } else if (S_EQ(oper, "<=")) {
            s_stree e_tmp1; s_stree e_tmp2;
            g_frame_level e_l1 = (loop_expr == 1) ? f_temporary : f_local, e_l2 = (loop_expr == 1) ? f_temporary : f_local;
            e_tmp1 = g_tmp(e_l1, d_bool);
            e_tmp2 = g_tmp(e_l2, d_bool);

            frame_level = e_l1;
            fprintf(out, "LT ");
            g_value(e_tmp1, 1); fprintf(out, " ");
            frame_level = l1;
            g_value(src1, s1); fprintf(out, " ");
            frame_level = l2;
            g_value(src2, s2); fprintf(out, "\n");
            
            frame_level = e_l2;
            fprintf(out, "EQ ");
            g_value(e_tmp2, 1); fprintf(out, " ");
            frame_level = l1;
            g_value(src1, s1); fprintf(out, " ");
            frame_level = l2;
            g_value(src2, s2); fprintf(out, "\n");

            frame_level = *src_frame;
            fprintf(out, "OR ");
            g_value(target, 1); fprintf(out, " ");
            frame_level = e_l1;
            g_value(e_tmp1, 1); fprintf(out, " ");
            frame_level = e_l2;
            g_value(e_tmp2, 1); fprintf(out, "\n");
        
            target->dtype = d_bool;
            return target;
        } else if (S_EQ(oper, ">=")) {
            s_stree e_tmp1; s_stree e_tmp2;
            g_frame_level e_l1 = (loop_expr == 1) ? f_temporary : f_local, e_l2 = (loop_expr == 1) ? f_temporary : f_local;
            e_tmp1 = g_tmp(e_l1, d_bool);
            e_tmp2 = g_tmp(e_l2, d_bool);

            frame_level = e_l1;
            fprintf(out, "GT ");
            g_value(e_tmp1, 1); fprintf(out, " ");
            frame_level = l1;
            g_value(src1, s1); fprintf(out, " ");
            frame_level = l2;
            g_value(src2, s2); fprintf(out, "\n");
            
            frame_level = e_l2;
            fprintf(out, "EQ ");
            g_value(e_tmp2, 1); fprintf(out, " ");
            frame_level = l1;
            g_value(src1, s1); fprintf(out, " ");
            frame_level = l2;
            g_value(src2, s2); fprintf(out, "\n");

            frame_level = *src_frame;
            fprintf(out, "OR ");
            g_value(target, 1); fprintf(out, " ");
            frame_level = e_l1;
            g_value(e_tmp1, 1); fprintf(out, " ");
            frame_level = e_l2;
            g_value(e_tmp2, 1); fprintf(out, "\n");
        
            target->dtype = d_bool;
            return target;
        } else if (S_EQ(oper, "AND") || S_EQ(oper, "&&")) {
            fprintf(out, "AND "); target->dtype = d_bool;
        } else if (S_EQ(oper, "OR") || S_EQ(oper, "||")) {
            fprintf(out, "OR "); target->dtype = d_bool;
        } else if (S_EQ(oper, "+")) {
            if (src1->dtype == d_string && src2->dtype == d_string) {
                fprintf(out, "CONCAT "); target->dtype = d_string;
            } else {
                fprintf(out, "ADD "); target->dtype = src1->dtype;
            }
        } else if (S_EQ(oper, "-")) {
            fprintf(out, "SUB "); target->dtype = src1->dtype;
        } else if (S_EQ(oper, "*")) {            
            fprintf(out, "MUL "); target->dtype = src1->dtype;
        } else if (S_EQ(oper, "/")) {
            if (src1->dtype == d_double && src2->dtype == d_int) {
                g_frame_level div_tmp_l = (loop_expr == 1) ? f_temporary : f_local;
                s_stree div_tmp = g_tmp(div_tmp_l, d_double);
                src2 = g_cast_full(int2float, div_tmp, div_tmp_l, 1, src2, l2, s2);
                s2 = 1;
                l2 = div_tmp_l;
                //src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);
            }
            else if (src1->dtype == d_int && src2->dtype == d_double) { 
                g_frame_level div_tmp_l = (loop_expr == 1) ? f_temporary : f_local;
                s_stree div_tmp = g_tmp(div_tmp_l, d_double);
                src1 = g_cast_full(int2float, div_tmp, div_tmp_l, 1, src1, l1, s1);
                s1 = 1;
                l1 = div_tmp_l;
                //src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);
            } else if (src1->dtype == d_int && src2->dtype == d_int) { 
                g_frame_level div_tmp_l = (loop_expr == 1) ? f_temporary : f_local;
                s_stree div_tmp = g_tmp(div_tmp_l, d_double);
                src1 = g_cast_full(int2float, div_tmp, div_tmp_l, 1, src1, l1, s1);
                s1 = 1;
                l1 = div_tmp_l;

                //src1 = g_cast(int2float, src1, l1, s1, &l1, &s1); 

                div_tmp_l = (loop_expr == 1) ? f_temporary : f_local;
                div_tmp = g_tmp(div_tmp_l, d_double);
                src2 = g_cast_full(int2float, div_tmp, div_tmp_l, 1, src2, l2, s2);
                s2 = 1;
                l2 = div_tmp_l;
                //src2 = g_cast(int2float, src2, l2, s2, &l2, &s2); 
            }

            //printf("> LOG (%s) > %d\n", oper, target->ntype);
            fprintf(out, "DIV "); 
            target->dtype = d_double;
        } else if (S_EQ(oper, "\\")) {
           // printf("> LOG (%s) > %d\n", oper, src1->ntype);
            

            // Pokud je některý double, zaokrouhli na int
            if (src1->ntype == n_var) {
                frame_level = l1; g_castif(float2r2eint, src1, s1);
            } else if (src1->dtype == d_double) {
                src1 = g_cast(float2r2eint, src1, l1, s1, &l1, &s1);
            } 
            if (src2->ntype == n_var) {
                frame_level = l2; g_castif(float2r2eint, src2, s2);
            } else if (src2->dtype == d_double) {
                src2 = g_cast(float2r2eint, src2, l2, s2, &l2, &s2);
            } 

            // Pro dělení, převeď na float
            if (src1->ntype == n_var) {
                frame_level = l1; g_castif(int2float, src1, s1);
            } else if (src1->dtype == d_int) {
                src1 = g_cast(int2float, src1, l1, s1, &l1, &s1);
            } 
            if (src2->ntype == n_var) {
                frame_level = l2; g_castif(int2float, src2, s2);
            } else if (src2->dtype == d_int) {
                src2 = g_cast(int2float, src2, l2, s2, &l2, &s2);
            } 

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

void g_cast_self(g_cast_type type, s_stree v, g_frame_level l, int system) {
    switch (type) {
        case int2float: {
            fprintf(out, "INT2FLOAT ");
            v->dtype = d_double;
            break;
        }

        case float2int: {            
            fprintf(out, "FLOAT2INT ");
            v->dtype = d_int;
            break;
        }

        case float2r2eint: {            
            fprintf(out, "FLOAT2R2EINT ");
            v->dtype = d_int;
            break;
        }

        case float2r2oint: {            
            fprintf(out, "FLOAT2R2OINT ");
            v->dtype = d_int;
            break;
        }

        case int2char: {            
            fprintf(out, "INT2CHAR ");
            v->dtype = d_string;
            break;
        }

        default: {
            return;
        }
    }

    s_stree node = v;
    if (node->ntype == n_f2i || node->ntype == n_i2f)
        node = v->rptr;

    g_frame_level bl = frame_level;
    frame_level = l;
    g_value(node, system);
    fprintf(out, " ");
    g_value(node, system);
    fprintf(out, "\n");
    frame_level = bl;
}

s_stree g_cast(g_cast_type ct, s_stree dst, g_frame_level dst_l, int dst_s, g_frame_level *t_l, int *t_s)
{
    s_stree target = dst;
    g_frame_level target_l = dst_l;
    g_frame_level bl = frame_level;
    int target_s = dst_s;

    if (target->ntype == n_const) {
        *t_l = (loop_expr == 1) ? f_temporary : f_local;
        *t_s = 1;
        target = g_tmp(*t_l, (ct == int2float) ? d_double : d_int);
        target_l = *t_l;
        target_s = *t_s;
        frame_level = bl;
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
    frame_level = (loop_expr == 1) ? f_temporary : f_local;  // při změně změnit LF@%s => TF@%s
    //frame_level = f_temporary;
    
    int cfs = cfs_next_id++;

    s_stree tmp = g_tmp(frame_level, d_string);     
    frame_level = bl;
    fprintf(out, "TYPE %s@%s ", (loop_expr == 1) ? "TF" : "LF",  tmp->value.v_string);
    g_value(tree, system);
    fprintf(out, "\n");
    fprintf(out, "JUMPIFNEQ cast_if_%d %s@%s string@", cfs, (loop_expr == 1) ? "TF" : "LF", tmp->value.v_string);

    if (type == int2char || type == int2float) {
        fprintf(out, "int");
    } else if (type == stri2int) {
        fprintf(out, "string");
    } else if (type == float2int || type == float2r2eint || type == float2r2oint) {
        fprintf(out, "float");
    }

    fprintf(out, "\n");
    g_cast(type, tree, frame_level, system, NULL, NULL);
    fprintf(out, "LABEL cast_if_%d\n", cfs);
}


s_stree g_castif_full(g_cast_type type, s_stree tree, int s, g_frame_level frame, s_stree cond, int cond_system, g_frame_level cond_frame, int *out_s, g_frame_level *out_l) 
{
    g_frame_level bl = frame_level;
    g_frame_level tl = (loop_expr == 1) ? f_temporary : f_local;
    frame_level = tl;  // při změně změnit LF@%s => TF@%s
    //frame_level = f_temporary;
    s_stree tmp = g_tmp(frame_level, d_string);       
    frame_level = bl;

    int cfs_id = cfs_next_id++;
    s_stree out_t = tree;
    *out_s = s;
    *out_l = frame_level;

    if (tree->ntype == n_const) {
        *out_s = 1;
        *out_l = (loop_expr == 1) ? f_temporary : f_local;
        out_t = g_tmp(*out_l, d_double);
        frame_level = *out_l;
        g_move(out_t, 1, tree, 0);
    }

    fprintf(out, "TYPE "); frame_level = tl; g_value(tmp, 1); fprintf(out, " "); frame_level = cond_frame; g_value(cond, cond_system); fprintf(out, "\n");

    if (type == int2float) {
        fprintf(out, "JUMPIFNEQ cast_if_%d ", cfs_id); frame_level = tl; g_value(tmp, 1); fprintf(out, " string@float\n");
        if (tree->ntype == n_const && tree->dtype == d_int) {
            
            fprintf(out, "INT2FLOAT ");
            g_value(out_t, *out_s); fprintf(out, " ");
            frame_level = frame; g_value(tree, s); fprintf(out, "\n");
        } else if (tree->ntype == n_var) {
            frame_level = frame;
            g_castif(int2float, tree, s);
        }
    } else if (type == float2int || type == float2r2eint || type == float2r2eint) {
        fprintf(out, "JUMPIFNEQ cast_if_%d ", cfs_id); frame_level = tl; g_value(tmp, 1); fprintf(out, " string@int\n");
        if (tree->ntype == n_const && tree->dtype == d_double) {            
            switch (type) {
                case float2int: fprintf(out, "FLOAT2INT "); break; 
                case float2r2eint: fprintf(out, "FLOAT2R2EINT "); break; 
                case float2r2oint: fprintf(out, "FLOAT2R2OINT "); break;
                default: break; 
            }
            g_value(out_t, *out_s); fprintf(out, " ");
            frame_level = frame; g_value(tree, s); fprintf(out, "\n");
        } else if (tree->ntype == n_var) {
            frame_level = frame;
            g_castif(type, tree, s);
        }
    }

    fprintf(out, "LABEL cast_if_%d\n", cfs_id);
    frame_level = bl;
    return out_t;
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

        switch (tree->lptr->dtype) {
            case d_int: {
                g_castif(float2r2eint, tree->lptr, 0);
                break;
            }
            case d_double: {
                g_castif(int2float, tree->lptr, 0);
                break;
            }
            default: {
                break;
            }
        }

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
            if (tree->rptr->ntype != n_const && tree->rptr->ntype != n_var) { // expr nebo call -> v returning node uloženo var
                g_castif(int2float, returning_node, ret_s);
            }
        } else if (fct->dtype == d_int) {
            if (tree->rptr->ntype == n_const) {
                ret_l = f_local;
                ret_s = 1;
                s_stree tmp = g_tmp(ret_l, tree->rptr->dtype);
                g_move(tmp, ret_s, tree->rptr, 0);
                g_cast_self(float2r2eint, tmp, ret_l, ret_s);
                returning_node = tmp;
            } else if (tree->rptr->ntype != n_var) // expr nebo call (var přetypovaná automaticky přs uzel vytvořeným v parseru)
                g_castif(float2r2eint, returning_node, ret_s);
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
        switch (fct->dtype)  {
            case d_bool : {
                fprintf(out, "PUSHS bool@true\n");
                break;
            }

            case d_int: {
                fprintf(out, "PUSHS int@0\n");
                break;
            }

            case d_double: {
                fprintf(out, "PUSHS float@0.0\n");
                break;
            }

            case d_string: {
                fprintf(out, "PUSHS string@\n");
                break;
            }

            default: {

            }
        }
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

    s_stree node = tree->lptr;
    int s = 0;
    g_frame_level l = frame_level;
    g_frame_level bl = frame_level;

    if (node->ntype == n_expr) {
        g_createTF();
        node = g_expression(node, &s, &l);   
    } else if (node->ntype == n_call) {
        g_call(node);
        char vname[10] = "tmp_";    
        sprintf(vname,"%s%d", vname, sysv_next_id++);       
        node = STcreateVar(vname, d_int);
        s = 1;
        l = f_local;
        //ret_l = f_temporary;
        frame_level = l;
        g_var(node, s);
        g_pop(node, s); 
    }

    frame_level = l;
    g_push(node, s);
    frame_level = bl;
}

void g_cond(s_stree tree)
{
    int cfs = cfs_next_id++;
    if (gs_push(stack, tree, cfs) == NULL) {
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

    fprintf(out, "JUMPIFNEQ conde_%d ", cfs);
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
    int cfs =  cfs_next_id++;
    if (gs_push(stack, tree, cfs) == NULL) {
        g_errno = G_ERROR_MEMORY | G_ERROR_STACK;
        return;
    }

    fprintf(out, "LABEL loopa_%d\n", cfs);
    
    s_stree r_var = tree->rptr;
    int s = 0;
    g_frame_level l = frame_level;
    g_frame_level bl = frame_level;
    if (tree->rptr->ntype == n_expr) {
       g_createTF();
       loop_expr = 1;
       r_var = g_expression(r_var, &s, &l);
    }

    frame_level = l;
    fprintf(out, "JUMPIFNEQ loope_%d ", cfs);
    g_value(r_var, s);
    fprintf(out, " bool@true\n");
    frame_level = bl;
}

void g_loop_end(s_stree tree)
{
    (void) tree;

    gStackItem item = gs_top(stack);
    
    loop_expr = 0;
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
                unsigned char *so = stringify(tree->value.v_string);
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


void g_buildIn() {
    g_buildInLength();
    g_buildInSubStr();
    g_buildInAsc();
    g_buildInChr();
}

void g_buildInLength() {
    char *fcname = "length";
    fprintf(out, "JUMP fcaf_%s\n\nLABEL fc_%s\n", fcname, fcname);
    fprintf(out, "CREATEFRAME\n");
    fprintf(out, "PUSHFRAME\n");

    fprintf(out, "DEFVAR LF@s\n");
    fprintf(out, "DEFVAR LF@l\n");
    fprintf(out, "POPS LF@s\n");
    fprintf(out, "STRLEN LF@l LF@s\n");
    fprintf(out, "PUSHS LF@l\n");

    fprintf(out, "LABEL fce_%s\n", fcname);
    fprintf(out, "POPFRAME\n");
    fprintf(out, "RETURN\n");
    fprintf(out, "LABEL fcaf_%s\n\n", fcname);
}

void g_buildInSubStr() { // SubStr(s As String, i As Integer, n As Integer) As String
    char *fcname = "substr";
    fprintf(out, "JUMP fcaf_%s\n\nLABEL fc_%s\n", fcname, fcname);
    fprintf(out, "CREATEFRAME\n");
    fprintf(out, "PUSHFRAME\n");

    fprintf(out, "DEFVAR LF@s\n");
    fprintf(out, "DEFVAR LF@i\n");
    fprintf(out, "DEFVAR LF@n\n");
    fprintf(out, "POPS LF@s\n");
    fprintf(out, "POPS LF@i\n");
    fprintf(out, "POPS LF@n\n");

    fprintf(out, "DEFVAR LF@in_ss_tmp1\n"); // typová kontrola pro i
    fprintf(out, "TYPE LF@in_ss_tmp1 LF@i\n");
    fprintf(out, "JUMPIFEQ in_ss_cast1 LF@in_ss_tmp1 string@int\n");
        fprintf(out, "FLOAT2R2EINT LF@i LF@i\n");
    fprintf(out, "LABEL in_ss_cast1\n");

    fprintf(out, "TYPE LF@in_ss_tmp1 LF@n\n"); // typová kontrola pro n
    fprintf(out, "JUMPIFEQ in_ss_cast2 LF@in_ss_tmp1 string@int\n");
        fprintf(out, "FLOAT2R2EINT LF@n LF@n\n");
    fprintf(out, "LABEL in_ss_cast2\n");

    fprintf(out, "JUMPIFEQ fce0_%s LF@s string@\n", fcname); // s je prázdné => return ""

    fprintf(out, "DEFVAR LF@in_ss_tmp2\n");
    fprintf(out, "LT LF@in_ss_tmp2 LF@i int@1\n");
    fprintf(out, "JUMPIFEQ fce0_%s LF@in_ss_tmp2 bool@true\n", fcname); // i <= 0 => return ""
    
    fprintf(out, "DEFVAR LF@l\n");
    fprintf(out, "STRLEN LF@l LF@s\n"); // l = length(s)
    fprintf(out, "DEFVAR LF@in_ss_tmp3\n");
    fprintf(out, "SUB  LF@in_ss_tmp3 LF@l LF@i\n"); // in_ss_tmp3 = l - i
    fprintf(out, "DEFVAR LF@in_ss_tmp4\n");
    fprintf(out, "GT LF@in_ss_tmp4 LF@n LF@in_ss_tmp3\n");
    fprintf(out, "JUMPIFNEQ in_ss_cond0 LF@in_ss_tmp4 bool@true\n"); // n > length(s) - i
        fprintf(out, "SUB LF@n LF@l LF@i\n");
        fprintf(out, "ADD LF@n LF@n int@1\n");
    fprintf(out, "LABEL in_ss_cond0\n");

    fprintf(out, "LT LF@in_ss_tmp4 LF@n int@0\n");
    fprintf(out, "JUMPIFNEQ in_ss_cond5 LF@in_ss_tmp4 bool@true\n"); // n < 0
        fprintf(out, "SUB LF@n LF@l LF@i\n");
        fprintf(out, "ADD LF@n LF@n int@1\n");
    fprintf(out, "LABEL in_ss_cond5\n");

    fprintf(out, "DEFVAR LF@result\n");
    fprintf(out, "DEFVAR LF@result_char\n");
    fprintf(out, "DEFVAR LF@in_ss_tmp5\n");
    fprintf(out, "SUB LF@i LF@i int@1\n");
    fprintf(out, "MOVE LF@result string@\n");
    fprintf(out, "LABEL in_ss_loop\n");
    fprintf(out, "SUB LF@n LF@n int@1\n"); // n = n - 1
    fprintf(out, "LT LF@in_ss_tmp5 LF@n int@0\n");
    fprintf(out, "JUMPIFEQ in_ss_loop_end LF@in_ss_tmp5 bool@true\n");

    fprintf(out, "GETCHAR LF@result_char LF@s LF@i\n");
    fprintf(out, "CONCAT LF@result LF@result LF@result_char\n");
    fprintf(out, "ADD LF@i LF@i int@1\n");

    fprintf(out, "JUMP in_ss_loop\n");
    fprintf(out, "LABEL in_ss_loop_end\n");

    fprintf(out, "PUSHS LF@result\n");

    fprintf(out, "JUMP fce_%s\n", fcname);
    
    fprintf(out, "LABEL fce0_%s\n", fcname);
    fprintf(out, "PUSHS string@\n");
    fprintf(out, "LABEL fce_%s\n", fcname);
    fprintf(out, "POPFRAME\n");
    fprintf(out, "RETURN\n");
    fprintf(out, "LABEL fcaf_%s\n\n", fcname);
}

void g_buildInAsc() {
    char *fcname = "asc";
    fprintf(out, "JUMP fcaf_%s\n\nLABEL fc_%s\n", fcname, fcname);
    fprintf(out, "CREATEFRAME\n");
    fprintf(out, "PUSHFRAME\n");

    fprintf(out, "DEFVAR LF@s\n");
    fprintf(out, "POPS LF@s\n");
    fprintf(out, "DEFVAR LF@i\n");
    fprintf(out, "POPS LF@i\n");
    fprintf(out, "DEFVAR LF@in_tmp1\n"); // typová kontrola float -> int pokud potřeba
    fprintf(out, "TYPE LF@in_tmp1 LF@i\n");
    fprintf(out, "JUMPIFEQ in_asc_cast1 LF@in_tmp1 string@int\n");
        fprintf(out, "FLOAT2R2EINT LF@i LF@i\n");
    fprintf(out, "LABEL in_asc_cast1\n");

    fprintf(out, "DEFVAR LF@l\n");
    fprintf(out, "STRLEN LF@l LF@s\n");
    fprintf(out, "DEFVAR LF@in_asc_tmp1\n");
    fprintf(out, "DEFVAR LF@in_asc_tmp2\n");
    fprintf(out, "GT LF@in_asc_tmp1 LF@i LF@l\n"); // i > délka
    fprintf(out, "LT LF@in_asc_tmp2 LF@i int@1\n"); // i < 1
    fprintf(out, "OR LF@in_asc_tmp1 LF@in_asc_tmp1 LF@in_asc_tmp2\n"); // (i > délka) || (i < 1)
    fprintf(out, "JUMPIFNEQ in_asc_cond1 LF@in_asc_tmp1 bool@true\n");
        fprintf(out, "PUSHS int@0\n");
        fprintf(out, "JUMP fce_%s\n", fcname);
    fprintf(out, "LABEL in_asc_cond1\n");
        fprintf(out, "SUB LF@i LF@i int@1\n");
        fprintf(out, "STRI2INT LF@in_asc_tmp1 LF@s LF@i\n");
        fprintf(out, "PUSHS LF@in_asc_tmp1\n");
    fprintf(out, "LABEL fce_%s\n", fcname);
    fprintf(out, "POPFRAME\n");
    fprintf(out, "RETURN\n");
    fprintf(out, "LABEL fcaf_%s\n\n", fcname);
}

void g_buildInChr() {
    char *fcname = "chr";
    fprintf(out, "JUMP fcaf_%s\n\nLABEL fc_%s\n", fcname, fcname);
    fprintf(out, "CREATEFRAME\n");
    fprintf(out, "PUSHFRAME\n");

    fprintf(out, "DEFVAR LF@i\n");
    fprintf(out, "POPS LF@i\n");
    fprintf(out, "DEFVAR LF@in_tmp1\n"); // typová kontrola float -> int pokud potřeba
    fprintf(out, "TYPE LF@in_tmp1 LF@i\n");
    fprintf(out, "JUMPIFEQ in_cast1 LF@in_tmp1 string@int\n");
    fprintf(out, "FLOAT2R2EINT LF@i LF@i\n");
    fprintf(out, "LABEL in_cast1\n");
    fprintf(out, "INT2CHAR LF@i LF@i\n");
    fprintf(out, "PUSHS LF@i\n");

    fprintf(out, "LABEL fce_%s\n", fcname);
    fprintf(out, "POPFRAME\n");
    fprintf(out, "RETURN\n");
    fprintf(out, "LABEL fcaf_%s\n\n", fcname);
}

s_stree g_tmp(g_frame_level frame, e_dstype type) 
{
    g_frame_level bl = frame_level;
    frame_level = frame;
    char vname[10] = "tmp_";    
    sprintf(vname,"%s%d", vname, sysv_next_id++);       
    s_stree tmp = STcreateVar(vname, type);
    g_var(tmp, 1); // tmp is always system
    frame_level = bl;
    return tmp;
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

// EXTERNAL //

int generate(s_list list, FILE *f)
{
    out = f;
    stack = gs_init();
    printHeader();

    for (int i = 0; i < list->size && !g_errno; i++) {
        processTree(Lget(&list, i));
    }

    g_buildIn();

    if (stack->top != NULL)
        g_errno = G_ERROR_STACK | G_ERROR_NOT_EMPTY;
    
    fprintf(out, "CLEARS\n");
    
    gs_destroy(stack);

    return g_errno;
}