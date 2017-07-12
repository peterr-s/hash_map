#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define HM_ERR_ALLOC 1
#define HM_W_NOTFOUND 2

#define DEFAULT_LOAD_FACTOR 0.75
#define DEFAULT_LEN 10

#define put hash_map_put
#define get hash_map_get
#define drop hash_map_drop

/* stdbool.h is not required by C89 */
typedef unsigned char bool;
#define false 0
#define true !false

typedef struct node node;
struct node
{
	void* key;
	void* value;
	node* next;
};

typedef struct hash_map hash_map;
struct hash_map
{
	node** table;
	unsigned long int(* hash_fn)(void* key);
	bool(* eq_fn)(void* p1, void* p2);
	unsigned long int table_len;
	float load_factor;
	unsigned long int element_ct;
};

short int hash_map_init(hash_map* map, unsigned long int(* hash_fn)(void* key), bool(* eq_fn)(void* p1, void* p2), unsigned long int start_len, float load_factor);
void hash_map_destroy(hash_map* map);

short int hash_map_put(hash_map* map, void* key, void* value);
short int hash_map_put_destroy(hash_map* map, void* key, void* value);
void* hash_map_get(hash_map* map, void* key);
short int hash_map_drop(hash_map* map, void* key);

short int hash_map_fast_put(hash_map* map, void* key, void* value);
short int hash_map_fast_put_destroy(hash_map* map, void* key, void* value);
void* hash_map_fast_get(hash_map* map, void* key);
short int hash_map_fast_drop(hash_map* map, void* key);

unsigned long int default_hash(void* key);
unsigned long int string_hash(void* key);

bool default_eq(void* p1, void* p2);
bool string_eq(void* p1, void* p2);

unsigned long int i, j;

#ifdef __cplusplus
}
#endif

#endif