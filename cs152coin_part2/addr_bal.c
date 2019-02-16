#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addr_bal.h"
#include "hmap.h"

typedef struct dbl {
    double v;
} dbl_t;

addr_bal_t* addr_bal_new()
{
    addr_bal_t* ab = calloc(1, sizeof(addr_bal_t));
    if (ab == NULL) {
      return NULL;
    }

    ab->hmap = hmap_new();
    if (ab->hmap == NULL) {
      free(ab);
      return NULL;
    }

    return ab;
}

static void free_dbl(const char* key, void* value)
{
    free(value);
}

void addr_bal_free(addr_bal_t* ab)
{
    hmap_foreach(&free_dbl, ab->hmap);
    hmap_free(ab->hmap);
    free(ab);
}

int get_balance(const char* addr, double* out_bal, addr_bal_t* ab)
{
    dbl_t* d = (dbl_t*)hmap_get(addr, ab->hmap);
    if (d == NULL) {
        *out_bal = 0;
        return 0;
    }

    *out_bal = d->v;
    return 1;
}

void set_balance(const char* addr, double bal, addr_bal_t* ab)
{
    dbl_t* d = (dbl_t*)hmap_get(addr, ab->hmap);
    if (d != NULL) {
        d->v = bal;
    } else {
        d = malloc(sizeof(dbl_t));
        if (d == NULL) {
            fprintf(stderr,"couldn't add to addr_bal\n");
        }
        
        d->v = bal;
        hmap_put(addr, d, ab->hmap);
    }
}

static void print_addr(const char* key, void* value)
{
    dbl_t* d = (dbl_t*)value;
    printf("Addr %s, Bal: %lf\n", key, d->v);
}

void addr_bal_print(addr_bal_t* ab)
{
    hmap_foreach(&print_addr, ab->hmap);
}