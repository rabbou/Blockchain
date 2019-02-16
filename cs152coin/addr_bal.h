#ifndef _ADDR_BAL_H
#define _ADDR_BAL_H


typedef struct bucket bucket;
struct bucket {
	char* addr;
	unsigned long int hash;
	double balance;
	bucket* next;
};

typedef struct addr_bal {
	bucket** buckets;
	size_t n_buckets;
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

/* return a pointer to a bucket where an address is located */
bucket* addr_bucket(const char* addr, addr_bal_t* ab);

/* compute hash code for given string */
unsigned long int good_hash(const char* addr);

/* build new list of buckets with given string at the head */
bucket* bucket_cons(const char* addr, unsigned long int hash, double bal, bucket* prev_head);

/* free the list nodes and all strings pointed to as well (deep free) */
void bucket_free(bucket* b);

#endif /* _HSET_H */
