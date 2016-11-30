#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tables.h"
#include "ast.h"

// Literals Table
// ----------------------------------------------------------------------------

#define LITERAL_MAX_SIZE 128
#define LITERALS_TABLE_MAX_SIZE 100

struct lit_table {
    char t[LITERALS_TABLE_MAX_SIZE][LITERAL_MAX_SIZE];
    int size;
};

LitTable* create_lit_table() {
    LitTable *lt = malloc(sizeof * lt);
    lt->size = 0;
    return lt;
}

int add_literal(LitTable* lt, char* s) {
    int i;
    for (i = 0; i < lt->size; i++) {
        if (strcmp(lt->t[i], s) == 0) {
            return i;
        }
    }
    strcpy(lt->t[lt->size], s);
    int old_side = lt->size;
    lt->size++;
    return old_side;
}

char* get_literal(LitTable* lt, int i) {
    return lt->t[i];
}

void print_lit_table(LitTable* lt) {
    int i;
    printf("Literals table:\n");
    for (i = 0; i < lt->size; i++) {
        printf("Entry %d -- %s\n", i, get_literal(lt, i));
    }
}

void free_lit_table(LitTable* lt) {
    free(lt);
}

// Symbols Table
// ----------------------------------------------------------------------------

#define SYMBOL_MAX_SIZE 128
#define SYMBOL_TABLE_MAX_SIZE 100

typedef struct {
  char name[SYMBOL_MAX_SIZE];
  int line;
  int arity;
  int scope;
  int offset;
  AST *pointer;
} Entry;

struct sym_table {
    Entry t[SYMBOL_TABLE_MAX_SIZE];
    int size;
};

SymTable* create_sym_table() {
    SymTable *st = malloc(sizeof * st);
    st->size = 0;
    return st;
}

int lookup_var(SymTable* st, char* s, int scope) {
    int i;
    for (i = 0; i < st->size; i++) {
        if (strcmp(st->t[i].name, s) == 0 && st->t[i].scope == scope)  {
            return i;
        }
    }
    return -1;
}

int lookup_func(SymTable* st, char* s) {
    int i;
    for (i = 0; i < st->size; i++) {
        if (strcmp(st->t[i].name, s) == 0)  {
            return i;
        }
    }
    return -1;
}

int add_func(SymTable* st, char* s, int line, int arity) {
    strcpy(st->t[st->size].name, s);
    st->t[st->size].line = line;
    int old_side = st->size;
    st->t[st->size].scope = -1;
    st->t[st->size].arity = arity;
    st->t[st->size].offset = 0;
    st->t[st->size].pointer = NULL;
    st->size++;
    return old_side;
}

int add_var(SymTable* st, char* s, int line, int scope) {
    strcpy(st->t[st->size].name, s);
    st->t[st->size].line = line;
    int old_side = st->size;
    st->t[st->size].arity = -1;
    st->t[st->size].scope = scope;
    st->t[st->size].offset = 0;
    st->t[st->size].pointer = NULL;
    st->size++;
    return old_side;
}

int get_arity(SymTable *st, int i){
    return st->t[i].arity;;
}

char* get_name(SymTable* st, int i) {
    return st->t[i].name;
}

int get_line(SymTable* st, int i) {
    return st->t[i].line;
}

int get_scope(SymTable* st, int i) {
    return st->t[i].scope;
}

int get_offset(SymTable* st, int i) {
    return st->t[i].offset;
}

void set_offset(SymTable* st, int i, int new_offset) {
    st->t[i].offset = new_offset;
}

void print_sym_table(SymTable* st) {
    int i;
    printf("Variables table:\n");
    for (i = 0; i < st->size; i++) {
         printf("Entry %d -- name: %s, line: %d, scope: %d\n", i, get_name(st, i), get_line(st, i), get_scope(st,i));
    }
}

void print_func_table(SymTable* st) {
    int i;
    printf("Functions table:\n");
    for (i = 0; i < st->size; i++) {
         printf("Entry %d -- name: %s, line: %d, arity: %d\n", i, get_name(st, i), get_line(st, i), get_arity(st,i));
    }
}

void free_sym_table(SymTable* st) {
    free(st);
}
