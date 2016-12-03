#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "tables.h"
#include "ast.h"

extern SymTable *st;
extern SymTable *ft;
extern LitTable *lt;
// Data stack -----------------------------------------------------------------

#define STACK_SIZE 100

int stack[STACK_SIZE];
int sp; // stack pointer
int fp; //frame pointer
int offset; //offset value

void push(int x)    // Insert into the stack
{
    stack[++sp] = x;
}

int pop()   // Remove and return the element of stack top
{
    return stack[sp--];
}

void init_stack() //Init stack
{
    int i;
    for (i = 0; i < STACK_SIZE; i++)
    {
        stack[i] = 0;
    }
    sp = -1;
    fp = 0;
    offset = 0;
}

void print_stack()
{
    printf("*** STACK: ");
    int i;
    for (i = 0; i <= sp; i++)
    {
        printf("%d ", stack[i]);
    }
    printf("\n");
}

// ----------------------------------------------------------------------------

// Variables memory -----------------------------------------------------------

#define MEM_SIZE 100

int mem[MEM_SIZE];

void store(int addr, int val)
{
    mem[addr] = val;
}

int load(int addr)
{
    return mem[addr];
}

void init_mem()
{
    int addr;
    for (addr = 0; addr < MEM_SIZE; addr++)
    {
        mem[addr] = 0;
    }
}

// ----------------------------------------------------------------------------

// #define TRACE
#ifdef TRACE
#define trace(msg) printf("TRACE: %s\n", msg)
#else
#define trace(msg)
#endif

void rec_run_ast(AST *ast);

void init_func_pointers(AST* ast)
{
    int i, size = get_child_count(ast);

    for(i = 0; i < size; i++)
    {
        AST *decl = get_child(ast, i);
        AST *header = get_child(decl, 0);
        AST *id = get_child(header, 1);
        set_pointer(ft, getPos(id), decl);
    }
}

void run_func_list(AST *ast)
{
    trace("func_list");

    init_func_pointers(ast);

    int size = get_child_count(ast);

    rec_run_ast(get_child(ast, size - 1)); //execute the last child that is the main function
}

void run_func_decl(AST *ast)
{
    trace("func_decl");

    int i, size = get_child_count(ast);

    for (i = 0; i < size; i++)
    {
        rec_run_ast(get_child(ast, i));
    }
}

void run_func_header(AST *ast)
{
    trace("func_header");

    int size = get_child_count(ast);

    rec_run_ast(get_child(ast, size - 1)); //execute the last child that is the param_list

}

void run_param_list(AST *ast)
{
    trace("param_list");

    int i, size = get_child_count(ast);

    for (i = 0; i < size; i++)
    {
        AST *child = get_child(ast, i);
        set_offset(st, getPos(child), fp + offset++);
        store(i, pop());
    }

    for(i = 0; i < size; i++)
        push(load(i));
}

void run_func_body(AST *ast)
{
    trace("func_body");

    int i, size = get_child_count(ast);

    for (i = 0; i < size; i++)
    {
        rec_run_ast(get_child(ast, i));
    }
}

void run_var_list(AST *ast)
{
    trace("var_list");

    int i, size = get_child_count(ast);

    for (i = 0; i < size; i++)
    {
        AST *child = get_child(ast, i);
        set_offset(st, getPos(child), fp + offset++);
        sp++;
    }
}

void run_block(AST *ast)
{
    trace("block");

    int i, size = get_child_count(ast);

    for (i = 0; i < size; i++)
    {
        rec_run_ast(get_child(ast, i));
    }
}

void run_input(AST *ast)
{
    trace("input");

    int num;
    scanf("%d", &num);
    push(num);
}

void run_output(AST *ast)
{
    trace("output");

    rec_run_ast(get_child(ast, 0));
    int x = pop();
    printf("%d", x);
}

void process_string(char* str)
{
    while(*str != '\0')
    {
        if(*str == '\"')
            str++;
        else if(*str == '\\')
        {
            switch(*(++str))
            {
                case 'n':
                    printf("\n");
                    str++;
                    break;
                case 't':
                    printf("\t");
                    str++;
                    break;
                case '\"':
                    str++;
                    break;
                case '0':
                    return;
            }
        }
        else{
            printf("%c", *str);
            str++;
        }
    }
}

void run_write(AST *ast)
{
    trace("write");
    rec_run_ast(get_child(ast, 0));
    int x = pop();
    process_string(get_literal(lt, x));
}

void run_assign(AST *ast)
{
    trace("assign");
    rec_run_ast(get_child(ast, 1));
    AST *child = get_child(ast, 0);
    int var_idx = get_offset(st, getPos(child));
    store(var_idx, pop());
}

void run_num(AST *ast)
{
    trace("num");
    push(getPos(ast));
}

void run_svar(AST *ast)
{
    trace("svar");
    push(load(get_offset(st, getPos(ast))));
}

void run_cvar(AST *ast)
{
    trace("cvar");

    int i, size = get_child_count(ast);

    for (i = 0; i < size; i++)
    {
        rec_run_ast(get_child(ast, i));
    }
}

void run_if(AST *ast)
{
    trace("if");
    rec_run_ast(get_child(ast, 0));
    int test = pop();
    if (test == 1)
    {
        rec_run_ast(get_child(ast, 1));
    } else if (test == 0 && get_child_count(ast) == 3)
    {
        rec_run_ast(get_child(ast, 2));
    }
}

void run_repeat(AST *ast)
{
    trace("repeat");
    int again = 1;
    while (again) {
        rec_run_ast(get_child(ast, 0)); // Run body.
        rec_run_ast(get_child(ast, 1)); // Run test.
        again = !pop();
    }
}

void run_read(AST *ast)
{
    trace("read");
    int x;
    printf("read: ");
    scanf("%d", &x);
    AST *child = get_child(ast, 0);
    int var_idx = get_offset(st, getPos(child));
    store(var_idx, x);
}

void run_string(AST *ast)
{
    trace("string");
    push(getPos(ast));
}

#define bin_op() \
    rec_run_ast(get_child(ast, 0)); \
    rec_run_ast(get_child(ast, 1)); \
    int r = pop(); \
    int l = pop()

void run_while(AST *ast)
{
    rec_run_ast(get_child(ast, 0)); //run test
    int test = pop(); //getting result of test
    while (test == 1)
    {
        rec_run_ast(get_child(ast, 1)); //run body
        rec_run_ast(get_child(ast, 0)); //run test
        test = pop(); //getting result of test
    }
}

void run_arg_list(AST *ast)
{
    int i, size = get_child_count(ast);

    for (i = size - 1; i >= 0; i--)
    {
        rec_run_ast(get_child(ast, i));
    }
}

void run_fcall(AST *ast)
{
    //saving information of current frame
    push(fp);
    fp = ++sp;
    offset = 0;

    //run func
    rec_run_ast(get_child(ast, 0));

    rec_run_ast(get_pointer(ft, getPos(ast)));

    //recovering last frame
    sp = --fp;
    fp = pop();
}

void run_return(AST *ast)
{
    rec_run_ast(ast);
}

void run_plus(AST *ast)
{
    trace("plus");
    bin_op();
    push(l + r);
}

void run_minus(AST *ast)
{
    trace("minus");
    bin_op();
    push(l - r);
}

void run_times(AST *ast)
{
    trace("times");
    bin_op();
    push(l * r);
}

void run_over(AST *ast)
{
    trace("over");
    bin_op();
    push((int) l / r);
}

void run_lt(AST *ast)
{
    trace("lt");
    bin_op();
    push(l < r);
}

void run_le(AST *ast)
{
    trace("le");
    bin_op();
    push(l <= r);
}

void run_gt(AST *ast)
{
    trace("gt");
    bin_op();
    push(l > r);
}

void run_ge(AST *ast)
{
    trace("ge");
    bin_op();
    push(l >= r);
}

void run_neq(AST *ast)
{
    trace("neq");
    bin_op();
    push(l != r);
}

void run_eq(AST *ast)
{
    trace("eq");
    bin_op();
    push(l == r);
}

void run_id(AST *ast)
{
    trace("id");
    int var_idx = get_offset(st, getPos(ast));;
    push(load(var_idx));
}

void rec_run_ast(AST *ast)
{
    switch(get_kind(ast))
    {   //Nodes
        case FUNC_LIST_NODE:
            run_func_list(ast);
            break;
        case FUNC_DECL_NODE:
            run_func_decl(ast);
            break;
        case FUNC_HEADER_NODE:
            run_func_header(ast);
            break;
        case PARAM_LIST_NODE:
            run_param_list(ast);
            break;
        case FUNC_BODY_NODE:
            run_func_body(ast);
            break;
        case VAR_LIST_NODE:
            run_var_list(ast);
            break;
        case BLOCK_NODE:
            run_block(ast);
            break;
        case INPUT_NODE:
            run_input(ast);
            break;
        case OUTPUT_NODE:
            run_output(ast);
            break;
        case WRITE_NODE:
            run_write(ast);
            break;
        case ASSIGN_NODE:
            run_assign(ast);
            break;
        case SVAR_NODE:
            run_svar(ast);
            break;
        case CVAR_NODE:
            run_cvar(ast);
            break;
        case IF_NODE:
            run_if(ast);
            break;
        case WHILE_NODE:
            run_while(ast);
            break;
        case FUNC_CALL_NODE:
            run_fcall(ast);
            break;
        case RETURN_NODE:
            run_return(ast);
            break;
        case ARG_LIST_NODE:
            run_arg_list(ast);
            break;
        case STRING_NODE:
            run_string(ast);
            break;
        //Arith Op Nodes
        case PLUS_NODE:
            run_plus(ast);
            break;
        case MINUS_NODE:
            run_minus(ast);
            break;
        case TIMES_NODE:
            run_times(ast);
            break;
        case OVER_NODE:
            run_over(ast);
            break;
        //Comp Op Nodes
        case LT_NODE:
            run_lt(ast);
            break;
        case LE_NODE:
            run_le(ast);
            break;
        case GT_NODE:
            run_gt(ast);
            break;
        case GE_NODE:
            run_ge(ast);
            break;
        case NEQ_NODE:
            run_neq(ast);
            break;
        case EQ_NODE:
            run_eq(ast);
            break;
        //Variable Nodes
       case NUM_NODE:
            run_num(ast);
            break;
        case ID_NODE:
            run_id(ast);
            break;
        default:
            fprintf(stderr, "Invalid kind: %s!\n", kind2str(get_kind(ast)));
            exit(1);
    }
}

void run_ast(AST *ast)
{
    init_stack();
    init_mem();
    rec_run_ast(ast);
}
