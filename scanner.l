/* 1º Trabalho de compiladores */
/* Aluno: Leonardo Santos Paulucio */
/* Scanner para Linguagem C-Minus */

/* Opcoes flex */
%option outfile="scanner.c"
%option noyywrap
%option nounput
%option noinput

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "ast.h"
    #include "tables.h"
    #include "parser.h"

    #define process_token(type) yylval = create_node(convertToNode(type)); return type

    void process(const char*);
	void process_error();
    NodeKind convertToNode(int i);
    extern LitTable *lt;
    extern SymTable *aux;
    extern int scope;
%}

/*Palavras Reservadas*/
ELSE "else"
IF "if"
INPUT "input"
INT "int"
OUTPUT "output"
RETURN "return"
VOID "void"
WHILE "while"
WRITE "write"
/*Operadores Aritmeticos e de comparacao*/
PLUS "+"
MINUS "-"
TIMES "*"
OVER "/"
LT "<"
LE "<="
GT ">"
GE ">="
EQ "=="
NEQ "!="
ASSIGN "="
/*Tokens*/
SEMI ";"
COMMA ","
LPAREN "("
RPAREN ")"
LBRACK "["
RBRACK "]"
LBRACE "{"
RBRACE "}"

ID [a-zA-Z]+[a-zA-Z0-9]*
NUM [0-9]+
STRING \".*\"

/* Comments are handled in the rules section. First we need to define a start condition. */
%x          L_COMMENT ML_COMMENT

%%
{ELSE} {process_token(ELSE);}
{IF} {process_token(IF);}
{INPUT} {process_token(INPUT);}
{INT} {process_token(INT);}
{OUTPUT} {process_token(OUTPUT);}
{RETURN} {process_token(RETURN);}
{VOID} {process_token(VOID);}
{WHILE} {process_token(WHILE);}
{WRITE} {process_token(WRITE);}

{PLUS} {process_token(PLUS);}
{MINUS} {process_token(MINUS);}
{TIMES} {process_token(TIMES);}
{OVER} {process_token(OVER);}
{LT} {process_token(LT);}
{LE} {process_token(LE);}
{GT} {process_token(GT);}
{GE} {process_token(GE);}
{EQ} {process_token(EQ);}
{NEQ} {process_token(NEQ);}
{ASSIGN} {process_token(ASSIGN);}

{SEMI} {return SEMI;}
{COMMA} {return COMMA;}
{LPAREN} {return LPAREN;}
{RPAREN} {return RPAREN;}
{LBRACK} {return LBRACK;}
{RBRACK} {return RBRACK;}
{LBRACE} {return LBRACE;}
{RBRACE} {return RBRACE;}
{NUM} {yylval = create_node(convertToNode(NUM)); setPos(yylval, atoi(yytext)); return NUM;}
{ID} {yylval = create_node(convertToNode(ID)); setPos(yylval, add_var(aux, yytext, yylineno, scope)); return ID;}
{STRING} {yylval = create_node(convertToNode(STRING)); setPos(yylval, add_literal(lt, yytext)); return STRING;}
"\n"            yylineno++; /* eat up a single line */

[ \t]+          /* eat up whitespace */

"//"                 BEGIN(L_COMMENT); /* line comments */
<L_COMMENT>[^\n]* /* eat anything that’s not a \n */
<L_COMMENT>"\n"    yylineno++; BEGIN(INITIAL);

"/*"                 BEGIN(ML_COMMENT); /* multi-line comments */
<ML_COMMENT>[^\n] /* eat anything that’s not a \n */
<ML_COMMENT>"\n"    yylineno++;
<ML_COMMENT>"*/"    BEGIN(INITIAL);

. {process_error();}

%%

NodeKind convertToNode(int token)
{
    switch(token)
    {
        case ELSE: return ELSE_NODE;
        case INT: return INT_NODE;
        case INPUT: return INPUT_NODE;
        case OUTPUT: return OUTPUT_NODE;
        case VOID: return VOID_NODE;
        case PLUS: return PLUS_NODE;
        case MINUS:  return MINUS_NODE;
        case TIMES:  return TIMES_NODE;
        case OVER:   return OVER_NODE;
        case LT: return LT_NODE;
        case LE: return LE_NODE;
        case GT: return GT_NODE;
        case GE: return GE_NODE;
        case EQ: return EQ_NODE;
        case NEQ: return NEQ_NODE;
        case NUM: return NUM_NODE;
        case ID: return ID_NODE;
        case STRING: return STRING_NODE;
        case IF: return IF_NODE;
        case RETURN: return RETURN_NODE;
        case WHILE: return WHILE_NODE;
        case WRITE: return WRITE_NODE;
        case ASSIGN: return ASSIGN_NODE;

        default: printf("Invalid node kind: %d!\n", token);
                return -1;
    }
}

void process(const char* token)
{
		printf("%d: %s -> %s\n",yylineno, yytext, token);
}

void process_error()
{
    printf("SCANNING ERROR (%d): Unknown symbol %c\n", yylineno, yytext[0]);
    exit(1);
}

/*int main()
{
    yylex();
    return 0;
}*/
