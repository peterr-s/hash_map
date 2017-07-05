#ifndef HASH_MAP_IO_H
#define HASH_MAP_IO_H

#include "hash_map.h"
#include <stdio.h>

#define HM_ERR_STREAM -1
#define HM_ERR_IO_HEAD 0

int hash_map_write(FILE* stream, hash_map* map, size_t key_sz, size_t value_sz);
int hash_map_read(FILE* stream, hash_map* map, size_t key_sz, size_t value_sz);
int hash_map_fast_read(FILE* stream, hash_map* map, size_t key_sz, size_t value_sz);

int hash_map_custom_write(FILE* stream, hash_map* map, int(* write_fn)(FILE* stream, void* key, void* value));
int hash_map_custom_read(FILE* stream, hash_map* map, int(* read_fn)(FILE* stream, void** key, void** value));

#endif
