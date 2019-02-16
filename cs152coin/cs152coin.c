#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
    unsigned int n = 0;
    trans_t* trans = calloc(1, sizeof(trans_t));
    if (!trans) return NULL;

    next_token();
    if (!read_string(&s)) {
        trans_free(trans);
        return NULL;
    }
    trans->id = strdup(s);
    next_token();
    next_token();
    if (!read_string(&s)) {
        trans_free(trans);
        return NULL;
    }
    while (!strcmp(s, "WITHDRAWAL")) {
        trans->inputs[n] = parse_dep_wd();
        n++;
    }
    trans->num_inputs = n;

    next_token();
    trans->output = parse_dep_wd();
    next_token();

    return trans;
}

block_t* parse_block()
{
    char* s = NULL;
    if (!read_string(&s)) return NULL;
    block_t* block = calloc(1, sizeof(block_t));
    if (!block) return NULL;

    next_token();
    block->miner = parse_dep_wd();
    block->trans = parse_trans();
    if (!read_string(&s)) {
        block_free(block);
        return NULL;
    }
    block->trans->next = NULL;
    trans_t *trans = block->trans;
    while (!strcmp(s, "BEGIN_TRANSACTION")) {
        trans->next = parse_trans();
        trans = trans->next;
    }
    trans->next = NULL;
    next_token();

    return block;
}

blockchain_t* parse_blockchain()
{
    char* s = NULL;
    if (!read_string(&s)) return NULL;
    blockchain_t* bc = calloc(1, sizeof(blockchain_t));
    if (!bc) return NULL;

    bc->blocks = parse_block();
    bc->blocks->next = NULL;
    block_t* block = bc->blocks;
    while (!strcmp(s, "BEGIN_BLOCK")) {
        block->next = parse_block();
        block = block->next;
    }
    block->next = NULL;

    return bc;
}

void dep_wd_free(dep_wd_t* dw)
{
    if (dw) free(dw->addr);
    free(dw);
}

void trans_free(trans_t* trans)
{
    free(trans->id);
    for (int i=0; i<5; i++) {
        dep_wd_free(trans->inputs[i]);
    }
    dep_wd_free(trans->output);
    free(trans);
}

void block_free(block_t* block)
{
    dep_wd_free(block->miner);
    trans_t* head = block->trans;
    while (head) {
        trans_t* temp = head;
        head = head->next;
        trans_free(temp);
    }
    free(block);
}

void blockchain_free(blockchain_t* bc)
{
    block_t* head = bc->blocks;
    while (head) {
        block_t* temp = head;
        head = head->next;
        block_free(temp); 
    }
    addr_bal_free(bc->addr_bal);
    dbl_spends_free(bc->dbl_spends);
    free(bc);
}

void compute_stats(blockchain_t* bc)
{
    unsigned long int i = 0, j = 0;
    double sum = 0;
    block_t* temp_b = bc->blocks;
    addr_bal_t* ab = addr_bal_new();
    dbl_spends_t* ds = dbl_spends_new();

    while (temp_b) {
        set_balance(temp_b->miner->addr, temp_b->miner->amount, ab);
        j++;
        trans_t* temp_tr = temp_b->trans;
        while (temp_tr) {
            for (int i=0; i<5; i++) {
                if (temp_tr->inputs[i]) {
                    set_balance(temp_tr->inputs[i]->addr,
                                            -temp_tr->inputs[i]->amount, ab);
                    
                    if (addr_bucket(temp_tr->inputs[i]->addr, ab)->balance < 0){
                        add_dbl_spend(temp_tr->id, temp_tr->inputs[i]->addr,ds);
                    }
                }
            }
            set_balance(temp_tr->output->addr, temp_tr->output->amount, ab);
            sum += temp_tr->output->amount;
            i++;
            temp_tr = temp_tr->next;
        }
        temp_b = temp_b->next;
    }
    bc->num_blocks = j;
    bc->num_trans = i;
    if (i) bc->avg_trans_value = sum/i;
    bc->addr_bal = ab;
    bc->dbl_spends = ds;
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
    
    for (int i = 1; i < argc; i++) {
        double bal = 0;
        if (get_balance(argv[i], &bal, bc->addr_bal)) {
            printf("Balance for address %s: %lf\n", argv[i], bal);
        } else {
            printf("Address %s: not found\n", argv[i]);
        }
    }
    
    blockchain_free(bc);
}