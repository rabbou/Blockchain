#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "deanon.h"
#include "hmap.h"
#include "cs152coin.h"

deanon_t* deanon_new()
{
    deanon_t* d = calloc(1, sizeof(deanon_t));;
    d->hmap = hmap_new();
    if (!d->hmap || !d) {
        fprintf(stderr, "deanon_new fail\n");
        exit(1);
    }
    d->bin = NULL;
    d->n_bin = 0;
    return d;
}

static void vertex_free(const char* key, void* value)
{
	vertex* vx = value;
	cluster_t* c = vx->neigh;
	while (c) {
		cluster_t* free = c;
		c = c->next;
		cluster_free(free);
	}
	free(vx);
}

	
static void bin_free(cluster_t* bin)	
{
    while (bin) {
        cluster_t* free = bin;
        bin = bin->next;
        cluster_free(free);
    }
}

void deanon_free(deanon_t* d)
{
    if (d) {
        for (size_t i=0; i<d->n_bin; i++) {
            bin_free(d->bin[i]);
        }
        free(d->bin);
        hmap_foreach(&vertex_free, d->hmap);
        hmap_free(d->hmap);
        free(d);
    }
}

void add_address(const char* addr, deanon_t* d)
{
    hmap_put(addr, vertex_new(NULL), d->hmap);
}

void add_associations(dep_wd_t* trans_inputs[], unsigned int num_inputs, deanon_t* d)
{
    for (int i=0; i<num_inputs; i++) {
        vertex* vx = hmap_get(trans_inputs[i]->addr, d->hmap);
        for (int j=0; j<num_inputs; j++) {
        	if (j!=i) vx->neigh = cluster_cons(vx->neigh,trans_inputs[j]->addr);
        }
    }
}

cluster_t* get_cluster(const char* addr, deanon_t* d)
{
    vertex* vx = hmap_get(addr, d->hmap);
    if (vx->full_cluster) return vx->full_cluster;
    if (!vx->neigh) return NULL;
	is* s = is_new();
  	push(s, addr);
	vx->visited++;
  	cluster_t* c = NULL;
	while (s->ns) {
    	c = cluster_cons(c, s->ns->addr);
    	vx = hmap_get(s->ns->addr, d->hmap);
    	cluster_t* neigh = vx->neigh;
    	pop(s);
	    while (neigh) {
	    	vx = hmap_get(neigh->addr, d->hmap);
	      	if (!vx->visited) {
	        	push(s, neigh->addr);
	        	vx->visited++;        
	      	}
	      	neigh = neigh->next;
	    }
	}
    add_as_full(c, d);
  	is_free(s);
  	return c;
}

cluster_t* cluster_new(const char* addr)
{
    cluster_t* c = malloc(sizeof(cluster_t));
    char* a = strdup(addr);
    if (!c || !a) {
        fprintf(stderr, "Malloc fail\n");
        return NULL;
    }
    c->addr = a;
    c->next = NULL;
    return c;
}

void cluster_free(cluster_t* c)
{
    if (c) {
        free(c->addr);
        free(c);
    }
}




vertex* vertex_new(cluster_t* c)
{
	vertex* vx = malloc(sizeof(vertex));
	if (!vx) {
		fprintf(stderr, "Malloc fail\n");
		exit(1);
	}
	vx->neigh = c;
	vx->visited = 0;
	vx->full_cluster = NULL;
	return vx;
}

cluster_t *cluster_cons(cluster_t *c, const char* addr)
{
    cluster_t* new_c = cluster_new(addr);
    new_c -> next = c;
    return new_c;
}

void add_as_full(cluster_t* c, deanon_t* d)
{
    cluster_t* temp = c;
    while (temp) {
    	vertex* vx = hmap_get(temp->addr, d->hmap);
    	vx->full_cluster = c;
    	temp = temp->next;
    }

    ++d->n_bin;
    d->bin = realloc(d->bin, d->n_bin*sizeof(cluster_t*));
    if (!d->bin) {
        fprintf(stderr, "Add to bin fail\n");
        exit(1);
    }
    d->bin[d->n_bin-1] = c;
}



is *is_new() 
{
	is *s = (is*)malloc(sizeof(is));
	if (s==NULL) {
		fprintf(stderr,"is_new: malloc failed\n");
		exit(1);
	}
	s->ns = NULL;
	return s;
}

void push(is *s, const char* addr)
{
	cluster_t* new = cluster_new(addr);
	new->next = s->ns;
	s->ns = new;
}

char* pop(is *s)
{
	char* addr = s->ns->addr;
	cluster_t* temp = s->ns;
	s->ns = s->ns->next;
    free(temp->addr);
	free(temp);
	return addr;
}

void is_free(is*s)
{
	if (s!=NULL) {
		cluster_free(s->ns);
		free(s);
	}
}