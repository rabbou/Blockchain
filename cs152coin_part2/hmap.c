#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmap.h"

static const size_t NUM_INITIAL_BUCKETS = 16;
static const double MAX_LOAD_FACTOR = 0.75;

static unsigned long int hash(const char* s)
{
    unsigned long int h = 17;
    while (*s != '\0') {
      h = 37*h + *s;
      s++;
    }
    return h;
}

static bucket_t* bucket_cons(const char* key, unsigned long int hash, void* value, bucket_t* prev_head)
{
    bucket_t* b = malloc(sizeof(bucket_t));
    char* an = strdup(key);
    if (b == NULL || an == NULL) {
        if (b != NULL) free(b);
        if (an != NULL) free(an);
        
        fprintf(stderr,"couldn't allocate a new bucket\n");
        return prev_head;
    }
    
    b->key = an;
    b->hash = hash;
    b->value = value; // shallow copy the value
    b->next = prev_head;
    return b;
}

static bucket_t* bucket_get(const char* key, unsigned long int hash, bucket_t* b)
{
    for (; b != NULL; b = b->next) {
        if (b->hash == hash && !strcmp(b->key, key)) {
                return b;
        }
    }
    return NULL;
}

static void bucket_free(bucket_t* b)
{
    while (b != NULL) {
        bucket_t* prev_b = b;
        b = b->next;
        free(prev_b->key);
        free(prev_b);
    }
}

static void bucket_foreach(bucket_t* b, void(*f)(const char*, void*))
{
    for (; b != NULL; b = b->next) {
        f(b->key, b->value);
    }
}


hmap_t* hmap_new()
{
    hmap_t* hm = calloc(1, sizeof(hmap_t));
    if (hm == NULL) {
      return NULL;
    }

    hm->n_buckets = NUM_INITIAL_BUCKETS;
    hm->buckets = calloc(hm->n_buckets, sizeof(bucket_t*)); /* initialzes array to 0 */
    if (hm->buckets == NULL) {
      free(hm);
      return NULL;
    }

    return hm;
}

void hmap_free(hmap_t* hm)
{
    for (size_t i = 0; i < hm->n_buckets; i++) {
        bucket_free(hm->buckets[i]);
    }
    free(hm->buckets);
    free(hm);
}

void* hmap_get(const char* key, hmap_t* hm)
{
    unsigned long int h = hash(key);
    size_t i = h % hm->n_buckets;
    
    bucket_t* b = bucket_get(key, h, hm->buckets[i]);
    if (b == NULL) {
        return NULL;
    }
    
    return b->value;
}

static void hmap_resize(hmap_t* hm)
{
    size_t new_n_buckets = hm->n_buckets * 2;
    bucket_t** new_buckets = calloc(new_n_buckets, sizeof(bucket_t*));
    if (new_buckets == NULL) return;
    
    for (size_t i = 0; i < hm->n_buckets; i++) {
        for (bucket_t* b = hm->buckets[i]; b != NULL; ) {
            bucket_t* nb = b->next;
            
            size_t j = b->hash % new_n_buckets;
            b->next = new_buckets[j];
            new_buckets[j] = b;
            
            b = nb;
        }
    }
    
    free(hm->buckets);
    hm->buckets = new_buckets;
    hm->n_buckets = new_n_buckets;
}

int hmap_put(const char* key, void* value, hmap_t* hm)
{
    unsigned long int h = hash(key);
    size_t i = h % hm->n_buckets;
    
    bucket_t* b = bucket_get(key, h, hm->buckets[i]);
    if (b != NULL) {
        return 0;
    }
    
    hm->buckets[i] = bucket_cons(key, h, value, hm->buckets[i]);
    hm->n_elts++;
    
    if ((double)hm->n_elts/(double)hm->n_buckets > MAX_LOAD_FACTOR) {
        hmap_resize(hm);
    }
    
    return 1;
}

void hmap_foreach(void(*f)(const char*, void*), hmap_t* hm)
{
    for (size_t i = 0; i < hm->n_buckets; i++) {
        bucket_foreach(hm->buckets[i], f);
    }
}