/* 3º Trabalho de compiladores */
/* Aluno: Leonardo Santos Paulucio */
/* Analisador Semantico para Linguagem C-Minus */

/* Options to bison */
// File name of generated parser.
%output "parser.c"
// Produces a 'parser.h'
%defines "parser.h"
// Give proper error messages when a syntax error is found.
%define parse.error verbose
// Enable LAC (lookahead correction) to improve syntax error handling.
%define parse.lac full

// Enable the trace option so that debugging is possible.
%define parse.trace

%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "tables.h"
#include "interpreter.h"

int yylex(void);
void yyerror(char const *s);
void check_var(int i, int scope);
void new_var(int i, int scope);
void new_func(int i, int arity);
void check_func(int i, int arity);

extern int yylineno;

AST *tree = NULL;    //AST
LitTable *lt = NULL; //Literals Table
SymTable *st = NULL; //Symbols Table
SymTable *aux = NULL;
SymTable *ft = NULL; //Functions table

int scope = 0;
int decl_arity = 0;
int call_arity = 0;
%}

%define api.value.type {AST*} // Type of variable yylval;

%token ELSE IF INPUT INT OUTPUT RETURN VOID WHILE WRITE PLUS MINUS TIMES OVER LT LE GT GE EQ NEQ ASSIGN SEMI COMMA LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE NUM ID STRING
%left LT LE GT GE EQ NEQ
%left PLUS MINUS  /* Ops associativos a esquerda. */
%left TIMES OVER  /* Mais para baixo maior precedencia. */

//Start symbol for the gramar
%start program

%%

program: func_decl_list                                  {tree = $1;}
;

func_decl_list: func_decl_list func_decl                 {$$ = $1; add_leaf($1, $2);}
 			|	func_decl                                {$$ = new_subtree(FUNC_LIST_NODE, 1, $1);}
;

func_decl: func_header func_body                         {$$ = new_subtree(FUNC_DECL_NODE, 2, $1, $2); scope++;}
;

func_header: ret_type ID LPAREN params RPAREN            {$$ = new_subtree(FUNC_HEADER_NODE, 3, $1, $2, $4); new_func(getPos($2), decl_arity); decl_arity = 0; setPos($2, lookup_func(ft, get_name(aux, getPos($2))));}
;

func_body: LBRACE opt_var_decl opt_stmt_list RBRACE      {$$ = new_subtree(FUNC_BODY_NODE, 2, $2, $3);}
;

opt_var_decl:	%empty                                   {$$ = new_subtree(VAR_LIST_NODE, 0);}
			|	var_decl_list                            {$$ = $1;}
;

opt_stmt_list:	%empty                                   {$$ = new_subtree(BLOCK_NODE, 0);}
			|	stmt_list                                {$$ = $1;}
;

ret_type: 	INT                                          {$$ = $1;}
		|	VOID                                         {$$ = $1;}
;

params:	VOID                                             {$$ = new_subtree(PARAM_LIST_NODE,0);}
	|	param_list                                       {$$ = $1;}
;

param_list:	param_list COMMA param                       {$$ = $1; add_leaf($1, $3);}
		|	param                                        {$$ = new_subtree(PARAM_LIST_NODE, 1, $1);}
;

param:	INT ID                                           {new_var(getPos($2), scope); decl_arity++; $$ = create_node_aux(SVAR_NODE, lookup_var(st, get_name(aux, getPos($2)), scope));}
	|	INT ID LBRACK RBRACK                             {new_var(getPos($2), scope); decl_arity++; $$ = create_node_aux(CVAR_NODE, lookup_var(st, get_name(aux, getPos($2)), scope));}
;

var_decl_list:	var_decl_list var_decl                   {$$ = $1; add_leaf($1, $2);}
			|	var_decl                                 {$$ = new_subtree(VAR_LIST_NODE, 1, $1);}
;

var_decl:	INT ID SEMI                                  {new_var(getPos($2), scope); $$ = create_node_aux(SVAR_NODE, lookup_var(st, get_name(aux, getPos($2)), scope)); set_tam(st, lookup_var(st, get_name(aux, getPos($2)), scope), 0);}
		|	INT ID LBRACK NUM RBRACK SEMI                {new_var(getPos($2), scope); $$ = create_node_aux(CVAR_NODE, lookup_var(st, get_name(aux, getPos($2)), scope)); add_leaf($$, $4); set_tam(st, lookup_var(st, get_name(aux, getPos($2)), scope), getPos($4));}
;

stmt_list: 	stmt_list stmt                               {$$ = $1; add_leaf($1, $2);}
		|	stmt                                         {$$ = new_subtree(BLOCK_NODE, 1, $1);}
;

stmt:	assign_stmt                                      {$$ = $1;}
	|	if_stmt                                          {$$ = $1;}
	|	while_stmt                                       {$$ = $1;}
	|	return_stmt                                      {$$ = $1;}
	|	func_call SEMI                                   {$$ = $1;}
;

assign_stmt:	lval ASSIGN arith_expr SEMI              {$$ = new_subtree(ASSIGN_NODE, 2, $1, $3);}
;

lval:	ID                                               {$$ = new_subtree(SVAR_NODE, 0); check_var(getPos($1), scope); setPos($$, lookup_var(st, get_name(aux, getPos($1)), scope));}
	|	ID LBRACK NUM RBRACK                             {$$ = new_subtree(CVAR_NODE, 1, $3); check_var(getPos($1), scope); setPos($$, lookup_var(st, get_name(aux, getPos($1)), scope));}
	|	ID LBRACK ID RBRACK                              {$$ = new_subtree(CVAR_NODE, 0); check_var(getPos($1), scope); check_var(getPos($3), scope); setPos($$, lookup_var(st, get_name(aux, getPos($1)), scope)); add_leaf($$, create_node_aux(SVAR_NODE, lookup_var(st, get_name(aux, getPos($3)), scope)));}
;

if_stmt:	IF LPAREN bool_expr RPAREN block             {$$ = new_subtree(IF_NODE, 2, $3, $5);}
		|	IF LPAREN bool_expr RPAREN block ELSE block  {$$ = new_subtree(IF_NODE, 3, $3, $5, $7);}
;

block:	LBRACE opt_stmt_list RBRACE                      {$$ = $2;}
;

while_stmt: WHILE LPAREN bool_expr RPAREN block          {$$ = new_subtree(WHILE_NODE, 2, $3, $5);}
;

return_stmt:	RETURN SEMI                              {$$ = new_subtree(RETURN_NODE, 0);}
			|	RETURN arith_expr SEMI                   {$$ = new_subtree(RETURN_NODE, 1, $2);}
;

func_call:	output_call                                  {$$ = $1;}
		|	write_call                                   {$$ = $1;}
		|	user_func_call                               {$$ = $1;}
;

input_call: INPUT LPAREN RPAREN                          {$$ = $1;}
;

output_call: OUTPUT LPAREN arith_expr RPAREN             {$$ = $1; add_leaf($1, $3);}
;

write_call: WRITE LPAREN STRING RPAREN                   {$$ = new_subtree(WRITE_NODE, 1, $3);}
;

user_func_call:	ID LPAREN opt_arg_list RPAREN            {$$ = new_subtree(FUNC_CALL_NODE, 1, $3); check_func(getPos($1), call_arity); call_arity = 0; setPos($$, lookup_func(ft, get_name(aux, getPos($1))));}
;

opt_arg_list:	%empty                                   {$$ = new_subtree(ARG_LIST_NODE, 0);}
			|	arg_list                                 {$$ = $1;}
;

arg_list: 	arg_list COMMA arith_expr                    {$$ = $1; add_leaf($1, $3); call_arity++;}
		|	arith_expr                                   {$$ = new_subtree(ARG_LIST_NODE, 1, $1); call_arity++;}
;

bool_expr:	arith_expr bool_op arith_expr                {$$ = $2; add_leaf($2, $1); add_leaf($2, $3);}
;

bool_op: 	LT                                           {$$ = $1;}
		|	LE                                           {$$ = $1;}
		|	GT                                           {$$ = $1;}
		|	GE                                           {$$ = $1;}
		|	EQ                                           {$$ = $1;}
		|	NEQ                                          {$$ = $1;}
;

arith_expr: LPAREN arith_expr RPAREN                     {$$ = $2;}
		|	lval                                         {$$ = $1;}
		|	input_call                                   {$$ = $1;}
		|	user_func_call                               {$$ = $1;}
        |   arith_expr PLUS arith_expr                   {$$ = $2; add_leaf($2, $1); add_leaf($2, $3);}
        |   arith_expr MINUS arith_expr                  {$$ = $2; add_leaf($2, $1); add_leaf($2, $3);}
        |   arith_expr TIMES arith_expr                  {$$ = $2; add_leaf($2, $1); add_leaf($2, $3);}
        |   arith_expr OVER arith_expr                   {$$ = $2; add_leaf($2, $1); add_leaf($2, $3);}
        |	NUM                                          {$$ = $1;}
;

%%


// /////////////////////////////////////// PARSER ERROR ///////////////////////////////////
void yyerror (char const *s)
{
	printf("PARSE ERROR (%d): %s\n", yylineno, s);
}

// ////////////////////////////////////// SEMANTIC ERROR //////////////////////////////////
void new_func(int i, int arity) //Inserting a new function in function table if it doesn't exist
{
    char* name = get_name(aux, i);
    int line = get_line(aux, i);
    int idx = lookup_func(ft, name);

    if (idx != -1) {
        printf("SEMANTIC ERROR (%d): function '%s' already declared at line %d.\n",
            line, name, get_line(ft, idx));
        exit(1);
    }
    add_func(ft, name, line, arity);
}

void check_func(int i, int arity) //check if function exist
{
    char* name = get_name(aux, i);
    int line = get_line(aux, i);
    int idx = lookup_func(ft, name);
    int declared_arity;

    if (idx == -1) {
        printf("SEMANTIC ERROR (%d): function '%s' was not declared.\n", line, name);
        exit(1);
    }

    declared_arity = get_arity(ft, idx);

    if(declared_arity != arity){
        printf("SEMANTIC ERROR (%d): function '%s' was called with %d arguments but declared with %d parameters.\n", line, name, arity, declared_arity);
        exit(1);
    }
}

void check_var(int i, int scope) //check if variable exist
{
    char* name = get_name(aux, i);
    int line = get_line(aux, i);
    int idx = lookup_var(st, name, scope);

    if (idx == -1) {
        printf("SEMANTIC ERROR (%d): variable '%s' was not declared.\n", line, name);
        exit(1);
    }
}

void new_var(int i, int scope)  //insert new variable on table and checking if it doesn't exist
{
    char* name = get_name(aux, i);
    int line = get_line(aux, i);
    int idx = lookup_var(st, name, scope);

    if (idx != -1) {
        printf("SEMANTIC ERROR (%d): variable '%s' already declared at line %d.\n",
            line, name, get_line(st, idx));
        exit(1);
    }

    add_var(st, name, line, scope);
}


int main()
{
    lt = create_lit_table(); //Creating literals table
    st = create_sym_table(); //Creating symbols table
    aux = create_sym_table(); //Creating auxiliar table
    ft = create_sym_table(); //Creating functions table

    //yydebug = 1; // Enter debug mode.

    if(!yyparse()) //If parser success
    {
        stdin = fopen(ctermid(NULL), "r");
        run_ast(tree); //Running interpreter
        //printf("PARSE SUCESSFUL!\n"); //print message
  	    //print_dot(tree); //print tree in dot format
    }

    //print_AST(tree); //print ast nodes
    //print_lit_table(lt); //print literals table
    //print_sym_table(st); //print symbols table
    //print_func_table(ft); //print functions table

    //free memory
    free_tree(tree);
    free_sym_table(st);
    free_sym_table(aux);
    free_sym_table(ft);
    free_lit_table(lt);
    return 0;
}
