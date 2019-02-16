#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dbl_spends.h"


dbl_spends_t* dbl_spends_new()
{
    dbl_spends_t* ds = malloc(sizeof(dbl_spends_t));
    if (!ds){
        fprintf(stderr,"Allocation fail\n");
        exit(1);
    }
    ds->next = NULL;
    return ds;
}

void dbl_spends_free(dbl_spends_t* ds)
{
    dbl_spends_t* temp;
    while (ds) {
        temp = ds;
        ds = ds->next;
        free(temp->addr);
        free(temp->trans_id);
        free(temp);
    }
}

void add_dbl_spend(const char* trans_id, const char* addr, dbl_spends_t* ds)
{
    if (ds->num_items) {
        dbl_spends_t* new = dbl_spends_new();
        new->trans_id = strdup(trans_id);
        new->addr = strdup(addr);

        dbl_spends_t* temp = ds;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = new;
        new->next = NULL;
    } else {
        ds->trans_id = strdup(trans_id);
        ds->addr = strdup(addr);
    }
    ++(ds->num_items);
}

void dbl_spends_show(dbl_spends_t* ds, FILE* f)
{
    printf("<Transaction id>                    <Address>\n");

    if (!ds->num_items) {
        printf("No double spends!\n");
        return;
    }

    dbl_spends_t* temp = ds;
    while (temp) {
        fprintf(f, "<%s>, <%s>\n", temp->trans_id, temp->addr);
        temp = temp->next;
    }
}
