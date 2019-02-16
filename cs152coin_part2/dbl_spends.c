#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dbl_spends.h"


dbl_spends_t* dbl_spends_new()
{
    dbl_spends_t* ds = calloc(1, sizeof(dbl_spends_t));
    if (ds == NULL) {
      return NULL;
    }
    
    ds->first = NULL;
    ds->last = NULL;
    return ds;
}

void dbl_spends_free(dbl_spends_t* ds)
{
    if (ds == NULL) {
        return;
    }
    
    for (dbs_t* d = ds->first; d != NULL; ) {
        dbs_t* x = d;
        d = d->next;
        
        free(x->trans_id);
        free(x->addr);
        free(x);
    }
    free(ds);
}

void add_dbl_spend(const char* trans_id, const char* addr, dbl_spends_t* ds)
{
    dbs_t* d = calloc(1, sizeof(dbs_t));
    if (d == NULL) {
        return;
    }
    
    d->trans_id = strdup(trans_id);
    d->addr = strdup(addr);
    d->next = NULL;
    
    if (ds->first == NULL) {
        ds->first = d;
    }
    
    if (ds->last != NULL) {
        ds->last->next = d;
    }
    ds->last = d;
}

void dbl_spends_show(dbl_spends_t* ds, FILE* f)
{
    for (dbs_t* d = ds->first; d != NULL; d = d->next) {
        fprintf(f, "%s, %s\n", d->trans_id, d->addr);
    }
}