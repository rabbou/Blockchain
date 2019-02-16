#ifndef _DBL_SPENDS_H
#define _DBL_SPENDS_H

#include <stdio.h>

typedef struct dbs {
    char* trans_id;
    char* addr;
    struct dbs* next;
} dbs_t;

typedef struct dbl_spends {
    dbs_t* first;
    dbs_t* last;
} dbl_spends_t;


// Returns a new heap-allocated data structure for tracking double spends.
dbl_spends_t* dbl_spends_new();

// Frees a heap-allocated data structure for tracking double spends.
void dbl_spends_free(dbl_spends_t* ds);

// Add a double spend entry consisting of the id transaction of the transaction
// containing the double spend and the address whose balance went negative.
void add_dbl_spend(const char* trans_id, const char* addr, dbl_spends_t* ds);

// Print all of the double spend entries IN THE ORDER THEY APPEAR IN THE BLOCKCHAIN
// to the file f. Output should be of the form:
// <trans_id>, <addr>
// <trans_id>, <addr>
// ...
void dbl_spends_show(dbl_spends_t* ds, FILE* f);

#endif