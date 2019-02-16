#ifndef _ADDR_BAL_H
#define _ADDR_BAL_H

typedef struct hmap hmap_t;

typedef struct addr_bal {
    hmap_t* hmap;
} addr_bal_t;


// Returns a new heap-allocated data structure for tracking the balance of each address.
addr_bal_t* addr_bal_new();

// Frees a heap-allocated data structure for tracking the balance of each address.
void addr_bal_free(addr_bal_t* ab);

// Get the balance associated with address addr and store the result in the double
// pointed to by out_bal.
// Returns 0 if the address couldn't be found and nonzero otherwise.
int get_balance(const char* addr, double* out_bal, addr_bal_t* ab);

// Set the balance associated with address addr to bal.
void set_balance(const char* addr, double bal, addr_bal_t* ab);

void addr_bal_print(addr_bal_t* ab);


#endif /* _ADDR_BAL_H */
