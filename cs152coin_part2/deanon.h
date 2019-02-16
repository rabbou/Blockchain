#ifndef _DEANON_H
#define _DEANON_H

typedef struct dep_wd dep_wd_t;
typedef struct hmap hmap_t;

// A linked list representation of a cluster of related addresses
typedef struct cluster {
    char* addr;
    struct cluster* next;
} cluster_t;

typedef struct {
	cluster_t* neigh;
  	int visited;

  	cluster_t* full_cluster;
} vertex;

typedef struct deanon {
    hmap_t* hmap;

	// storage for clusters made in get_clusters, in order to make freeing easier
    cluster_t** bin;
    size_t n_bin;
} deanon_t;


// Returns a new heap-allocated data structure used to facilitate deanonimization
deanon_t* deanon_new();

// Frees a heap-allocated deanonimization data structure
void deanon_free(deanon_t* d);

// Adds an address addr to the deanonimization data structure
void add_address(const char* addr, deanon_t* d);

// Record that all of the supplied transaction inputs are associated with each other.
void add_associations(dep_wd_t* trans_inputs[], unsigned int num_inputs, deanon_t* d);

// Returns the cluster of addresses associated with the supplied address.
// A cluster is a linked list of cluster elements.
//
// IMPORTANT: If the supplied address belongs to a cluster that has never been returned before,
// get_cluster is allowed to take O(a + b) time where a is the total number of addresses
// and b is the number of pairs of addresses that have at some point appeared together as
// inputs to a transaction. 
//
// On the other hand, if the supplied address belongs to a cluster that has been returned before,
// then get_cluster should take constant -- i.e., O(1) -- time on average.
cluster_t* get_cluster(const char* addr, deanon_t* d);

// Returns a new heap-allocated cluster element that stores the supplied address.
// Makes a deep copy of the address string.
cluster_t* cluster_new(const char* addr);

// Frees a heap-allocated cluster element
void cluster_free(cluster_t* c);

// Allocs a new vector struct
vertex* vertex_new(cluster_t* c);

/* Add an address to the cluster if it is not already in it */
cluster_t *cluster_cons(cluster_t *c, const char* addr);

/* Add a cluster to the bin */
void add_as_full(cluster_t* c, deanon_t* d);

/* stack struct and functions stolen from hw6 */
typedef struct {
  cluster_t* ns;
} is;

is *is_new();

void push(is *s, const char* addr);

char* pop(is *s);

void is_free(is *s);

#endif