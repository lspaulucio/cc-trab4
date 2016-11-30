#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ast.h"

struct node
{
    // char* data_text;
    NodeKind type;
    int pos;
    int num_nodes;
    int alocated_nodes;
    struct node **nodes;
};

AST* create_node(int type)
{
    AST* node = (AST*) malloc(sizeof(struct node));

    // node->data_text = (char*) malloc(strlen(data) + 1);
    // strcpy(node->data_text, data);
    node->type = type;
    node->pos = -1;
    node->num_nodes = 0;
    node->alocated_nodes = 0;
    node->nodes = NULL;
    return node;
}

AST* create_node_aux(int type, int pos)
{
    AST* node = (AST*) malloc(sizeof(struct node));

    // node->data_text = (char*) malloc(strlen(data) + 1);
    // strcpy(node->data_text, data);
    node->type = type;
    node->pos = pos;
    node->num_nodes = 0;
    node->alocated_nodes = 0;
    node->nodes = NULL;
    return node;
}

void setPos(AST* node, int p)
{
    node->pos = p;
}

int getPos(AST* node)
{
    return node->pos;
}

int get_child_count(AST *node)
{
    return node->num_nodes;
}

AST* get_child(AST *node, int id)
{
    return node->nodes[id];
}

NodeKind get_kind(AST *node)
{
    return node->type;
}

// char* getName(AST* node)
// {
//     return node->data_text;
// }

void add_leaf(AST *node, AST *leaf)
{
	if(node->num_nodes >= node->alocated_nodes)
	{
      	node->nodes = realloc(node->nodes, sizeof(AST*) * ++(node->alocated_nodes));
      	node->nodes[node->num_nodes++] = leaf;
	}
    	else
		node->nodes[node->num_nodes++] = leaf;
}

AST* new_subtree(int type, int cnt_nodes, ...)
{
    int i;
    va_list nodes_list;

    AST *node = create_node(type);

    //Aloca memoria para os n nodes passados como parametros
    node->nodes = (AST**) malloc(sizeof(AST*) * cnt_nodes);

    node->alocated_nodes = cnt_nodes;

    va_start(nodes_list, cnt_nodes);

    for(i = 0; i < cnt_nodes; i++)
        add_leaf(node, va_arg(nodes_list, AST*));

    va_end(nodes_list);

    return node;
}

const char* STRING_NODEKIND[] =
{
    "else",
    "int",
    "input",
    "output",
    "void",
    "+",
    "-",
    "*",
    "/",
    "<",
    "<=",
    ">",
    ">=",
    "==",
    "!=",
    "num",
    "id",
    "string",
    ";",
    ",",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",

    "program",
    "func_list",
    "func_decl",
    "func_header",
    "func_body",
    "var_list",
    "block",
    "param_list",
    "param",
    "=",
    "if",
    "while",
    "return",
    "write",
    "fcall",
    "arg_list",
    "var_decl",
    "lval",
    "svar",
    "cvar"
};

void print_node(AST *node, int level)
{
    int i;

    printf("%d: Node -- Addr: %p -- Text: %s -- Count: %d\n", level, node, STRING_NODEKIND[node->type], node->num_nodes);

    for(i = 0; i < node->alocated_nodes; i++)
        print_node(node->nodes[i], level + 1);

}

void print_AST(AST *tree)
{
    print_node(tree, 0);
}

void free_tree(AST *tree)
{
    int i;

    if (tree != NULL)
    {
        for(i=0; i < tree->alocated_nodes; i++)
        {
            free_tree(tree->nodes[i]);
        }
        // free(tree->data_text);
        free(tree->nodes);
        free(tree);
    }
}

// Dot output.

int nr;

int print_node_dot(AST *node)
{
    int my_nr = nr++;
    int i;
    char s[50];

    node2str(node, s);
    printf("node%d[label=\"%s\"];\n", my_nr, s);

    for (i = 0; i < node->alocated_nodes; i++)
    {
        int child_nr = print_node_dot(node->nodes[i]);
        printf("node%d -> node%d;\n", my_nr, child_nr);
    }

    return my_nr;
}

void print_dot(AST *tree)
{
    nr = 0;
    printf("digraph {\ngraph [ordering=\"out\"];\n");
    print_node_dot(tree);
    printf("}\n");
}

void node2str(AST *node, char *s)
{
    switch(node->type)
    {
        case NUM_NODE: sprintf(s, "%s, %d", STRING_NODEKIND[node->type], node->pos); break;
        case SVAR_NODE: sprintf(s, "%s, %d", STRING_NODEKIND[node->type], node->pos); break;
        case CVAR_NODE: sprintf(s, "%s, %d", STRING_NODEKIND[node->type], node->pos); break;
        case ID_NODE: sprintf(s, "%s, %d", STRING_NODEKIND[node->type], node->pos); break;
        case STRING_NODE: sprintf(s, "%s, %d", STRING_NODEKIND[node->type], node->pos); break;
        case FUNC_CALL_NODE: sprintf(s, "%s, %d", STRING_NODEKIND[node->type], node->pos); break;


        default:
            sprintf(s, "%s", STRING_NODEKIND[node->type]);
    }
}

const char* kind2str(NodeKind type)
{
    return STRING_NODEKIND[type];
}
