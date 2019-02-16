#ifndef _HMAP_H
#define _HMAP_H

typedef struct bucket {
  char* key;
  unsigned long int hash;
  void* value;
  
  struct bucket* next;
} bucket_t;

typedef struct hmap {
    bucket_t** buckets; /* an array of pointers to buckets */
    size_t n_buckets;
    size_t n_elts;
} hmap_t;


// Returns a new heap-allocated hash map.
hmap_t* hmap_new();

// Frees a heap-allocated hash map.
void hmap_free(hmap_t* hm);

// Returns a pointer to a value associated with key k or NULL
// if the key was not found.
void* hmap_get(const char* key, hmap_t* hm);

// Adds a key-value pair to the hash map.
// Returns nonzero if the value was successfully inserted.
// If the has map already contains a value associated with the key, the value
// is not inserted, and this function returns 0.
int hmap_put(const char* key, void* value, hmap_t* hm);

// Calls the supplied function f on every key-value pair in the hash map.
// f is given pointers to the key and the value as arguments.
void hmap_foreach(void(*f)(const char*, void*), hmap_t* hm);

#endif /* _HMAP_H */
