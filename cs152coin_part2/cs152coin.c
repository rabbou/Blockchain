#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "cs152coin.h"


// This function is already implemented as an example of how to use the parser
dep_wd_t* parse_dep_wd()
{
    char* s = NULL;
    double d = 0;
    
    if (!read_string(&s)) return NULL;
    dep_wd_type type;
    if (!strcmp(s, "DEPOSIT")) {
        type = DEPOSIT;
    } else if (!strcmp(s, "WITHDRAWAL")) {
        type = WITHDRAWAL;
    } else {
        return NULL;
    }
    next_token();
    
    dep_wd_t* dw = calloc(1, sizeof(dep_wd_t));
    if (dw == NULL) {
        return NULL;
    }
    dw->type = type;
    
    if (!read_string(&s)) {
        dep_wd_free(dw);
        return NULL;
    }
    dw->addr = strdup(s);
    next_token();
    
    if (!read_double(&d)) {
        dep_wd_free(dw);
        return NULL;
    }
    dw->amount = d;
    next_token();
    
    return dw;
}

trans_t* parse_trans()
{
    char* s = NULL;
    
    if (!read_string(&s)) return NULL;
    if (strcmp(s, "BEGIN_TRANSACTION")) return NULL;
    next_token();
    
    trans_t* trans = calloc(1, sizeof(trans_t));
    if (trans == NULL) {
        return NULL;
    }
    
    if (!read_string(&s)) {
        trans_free(trans);
        return NULL;
    }
    trans->id = strdup(s);
    next_token();
    
    if (!read_string(&s)) {
        trans_free(trans);
        return NULL;
    }
    if (strcmp(s, "BEGIN_INPUTS")) {
        trans_free(trans);
        return NULL;
    }
    next_token();
    
    int i = 0;
    for (; i < 5; i++) {
        dep_wd_t* dw = parse_dep_wd();
        if (dw == NULL) break;
        
        trans->inputs[i] = dw;
        trans->num_inputs++;
    }
    
    if (!read_string(&s)) {
        trans_free(trans);
        return NULL;
    }
    if (strcmp(s, "END_INPUTS")) {
        trans_free(trans);
        return NULL;
    }
    next_token();
    
    trans->output = parse_dep_wd();
    
    if (!read_string(&s)) {
        trans_free(trans);
        return NULL;
    }
    if (strcmp(s, "END_TRANSACTION")) {
        trans_free(trans);
        return NULL;
    }
    next_token();
    
    return trans;
}

block_t* parse_block()
{
    char* s = NULL;
    
    if (!read_string(&s)) return NULL;
    if (strcmp(s, "BEGIN_BLOCK")) return NULL;
    next_token();
    
    block_t* block = calloc(1, sizeof(block_t));
    if (block == NULL) {
        return NULL;
    }
    
    block->miner = parse_dep_wd();
    if (block->miner == NULL) {
        block_free(block);
        return NULL;
    }
    
    trans_t* trans = parse_trans();
    block->trans = trans;
    
    while (trans != NULL) {
        trans_t* t = parse_trans();
        trans->next = t;
        trans = t;
    }
    
    if (!read_string(&s)) {
        block_free(block);
        return NULL;
    }
    if (strcmp(s, "END_BLOCK")) {
        block_free(block);
        return NULL;
    }
    next_token();
    
    return block;
}

blockchain_t* parse_blockchain()
{
    blockchain_t* bc = calloc(1, sizeof(blockchain_t));
    if (bc == NULL) {
        return NULL;
    }
    
    block_t* block = parse_block();
    bc->blocks = block;
    
    while (block != NULL) {
        block_t* b = parse_block();
        block->next = b;
        block = b;
    }
    
    return bc;
}

void dep_wd_free(dep_wd_t* dw)
{
    if (dw == NULL) {
        return;
    }
    
    if (dw->addr != NULL) {
        free(dw->addr);
    }
    free(dw);
}

void trans_free(trans_t* trans)
{
    if (trans == NULL) {
        return;
    }
    
    for (int i = 0; i < trans->num_inputs; i++) {
        dep_wd_free(trans->inputs[i]);
    }
    dep_wd_free(trans->output);
    
    if (trans->id != NULL) {
        free(trans->id);
    }
    free(trans);
}

void block_free(block_t* block)
{
    if (block == NULL) {
        return;
    }
    
    for (trans_t* t = block->trans; t != NULL; ) {
        trans_t* x = t;
        t = t->next;
        trans_free(x);
    }
    dep_wd_free(block->miner);
    free(block);
}

void blockchain_free(blockchain_t* bc)
{
    for (block_t* b = bc->blocks; b != NULL; ) {
        block_t* x = b;
        b = b->next;
        block_free(x);
    }
    addr_bal_free(bc->addr_bal);
    dbl_spends_free(bc->dbl_spends);
    deanon_free(bc->deanon);
    free(bc);
}

static double update_bal(dep_wd_t* dw, addr_bal_t* ab)
{
    double bal = 0;
    get_balance(dw->addr, &bal, ab);
    
    if (dw->type == DEPOSIT) {
        bal += dw->amount;
    } else {
        bal -= dw->amount;
    }
    
    set_balance(dw->addr, bal, ab);
    return bal;
}

void compute_stats(blockchain_t* bc)
{
    bc->addr_bal = addr_bal_new();
    bc->dbl_spends = dbl_spends_new();
    
    for (block_t* b = bc->blocks; b != NULL; b = b->next) {
        bc->num_blocks++;
        update_bal(b->miner, bc->addr_bal);
        
        for (trans_t* t = b->trans; t != NULL; t = t->next) {
            bc->num_trans++;
            
            for (unsigned int i = 0; i < t->num_inputs; i++) {
                dep_wd_t* dw = t->inputs[i];
                double new_bal = update_bal(dw, bc->addr_bal);
                if (new_bal < 0) {
                    add_dbl_spend(t->id, dw->addr, bc->dbl_spends);
                }
            }
            update_bal(t->output, bc->addr_bal);
            bc->avg_trans_value += t->output->amount;
        }
    }
    
    bc->avg_trans_value /= (double)bc->num_trans;
}


// =========================
// Deanonimization functions
// =========================

void deanon_preprocess(blockchain_t* bc)
{
    deanon_t* d = deanon_new();
    for (block_t* b = bc->blocks; b != NULL; b = b->next) {
        for (trans_t* t = b->trans; t != NULL; t = t->next) {
            for (unsigned int i = 0; i < t->num_inputs; i++) {
                if (!hmap_get(t->inputs[i]->addr, d->hmap)) {
                    add_address(t->inputs[i]->addr, d);
                }
            }
            if (t->num_inputs > 1) {
                add_associations(t->inputs, t->num_inputs, d);
            }
        }
    }
    bc->deanon = d;
}

unsigned int cluster_size(cluster_t* cluster)
{
    unsigned int size = 0;
    cluster_t* temp = cluster;
    while (temp) {
        size++;
        temp = temp->next;
    }
    return size;
}

double cluster_balance(cluster_t* cluster, blockchain_t* bc)
{
    double c_bal = 0;
    cluster_t* temp = cluster;
    while (temp) {
        double bal = 0;
        get_balance(temp->addr, &bal, bc->addr_bal);
        c_bal += bal;
        temp = temp->next;
    }
    return c_bal;
}

static void print_stats(blockchain_t* bc)
{
    if (bc == NULL) return;
    
    printf("Number of blocks: %lu\n", bc->num_blocks);
    printf("Number of transactions: %lu\n", bc->num_trans);
    printf("Average transaction value: %lf\n", bc->avg_trans_value);
    printf("\n");
    
    printf("Double spends:\n");
    dbl_spends_show(bc->dbl_spends, stdout);
    printf("\n");
}

int main(int argc, char* argv[])
{
    init_parser();
    blockchain_t* bc = parse_blockchain();
    
    compute_stats(bc);
    print_stats(bc);
    
    deanon_preprocess(bc);
    
    for (int i = 1; i < argc; i++) {
        const char* addr = argv[i];
        
        double bal = 0;
        if (get_balance(addr, &bal, bc->addr_bal)) {
            printf("Address: %s\n", addr);
            printf("Balance: %lf\n", bal);
            
            cluster_t* cluster = get_cluster(addr, bc->deanon);
            if (cluster != NULL) {
                printf("Cluster size: %u\n", cluster_size(cluster));
                printf("Cluster balance: %lf\n", cluster_balance(cluster, bc));
                printf("Addresses in cluster:\n");
                for (cluster_t* c = cluster; c != NULL; c = c->next) {
                    printf("%s\n", c->addr);
                }
            }
        } else {
            printf("Address %s: not found\n", addr);
        }
        
        printf("\n");
    }
    
    blockchain_free(bc);
}