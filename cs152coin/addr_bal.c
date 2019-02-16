#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addr_bal.h"

addr_bal_t* addr_bal_new()
{
    addr_bal_t* ab = malloc(sizeof(addr_bal_t));
    bucket** bs = malloc(100*sizeof(bucket*));
    if (!ab || !bs){
        fprintf(stderr,"Allocation fail\n");
        exit(1);
    }
    for (int i=0; i<100; i++){
        bs[i] = NULL;
    }
    ab -> buckets = bs;
    ab -> n_buckets = 100;
    return ab;
}

void addr_bal_free(addr_bal_t* ab)
{
    for (int i = (ab -> n_buckets - 1); i >= 0; i--) {
        bucket_free(ab -> buckets[i]);
    }
    free(ab->buckets);
    free(ab);
}

int get_balance(const char* addr, double* out_bal, addr_bal_t* ab)
{
    if (!addr_bucket(addr, ab)) return 0;
    unsigned long int hash = good_hash(addr);
    int index = hash % ab->n_buckets;
    bucket* temp = ab->buckets[index];

    while(temp) {
        if (temp->hash == hash && !strcmp(temp->addr, addr)) {
            *out_bal = temp->balance;
            return -1;
        }
        temp = temp->next;
    }
    return 0;
}

void set_balance(const char* addr, double bal, addr_bal_t* ab)
{
    unsigned long int hash = good_hash(addr);
    int index = hash % ab->n_buckets;

    if (!addr_bucket(addr, ab)) {
        ab -> buckets[index] = bucket_cons(addr, hash, bal, ab->buckets[index]);
    } else {
        bucket* b = addr_bucket(addr, ab);
        b->balance += bal;
    }
}

bucket* addr_bucket(const char* addr, addr_bal_t* ab)
{
    unsigned long int hash = good_hash(addr);
    int index = hash % ab->n_buckets;
    bucket* temp = ab->buckets[index];

    while(temp) {
        if (temp->hash == hash && !strcmp(temp->addr, addr)) return temp;
        temp = temp->next;
    }
    return NULL;
}

unsigned long int good_hash(const char* addr)
{
    unsigned long int res = 17;
    int i = 0;
    while (addr[i]) {
        res = 37 * res + addr[i++];
    }
    return res;
}

bucket* bucket_cons(const char* addr, unsigned long int hash, double bal,
                                                            bucket* prev_head)
{
    bucket *b = malloc(sizeof(bucket));
    if (!b){
        fprintf(stderr,"Allocation fail\n");
        exit(1);
    }
    b -> next = prev_head;
    b -> addr = strdup(addr);
    b -> balance = bal;
    b -> hash = hash;
    return b;
}

void bucket_free(bucket* b)
{
    bucket *temp;
    while (b) {
        temp = b;
        b = b -> next;
        free(temp -> addr);
        free(temp);
    }
}