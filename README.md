### hash_map.h

I couldn't find a general-purpose cross-platform C hash map implementation and needed one for a project I'm working on, so I wrote one. If it can be of use to you, have at it.

Everything is absolutely standards compliant C89 with no assumptions about implementation-defined behavior. It will also compile as C99 or C11.

##### Structure

This file defines a `hash_map` structure and basic methods to manipulate it. The hash map works entirely with void pointers, so it does not create additional copies of data that is added to it or destroy elements that are removed.

`hash_map_init` takes a pointer to a hash map, a pointer to a hash function, a pointer to an equality function, a start size for the internal table, and a load factor. On success it returns `0`. If it encounters an error when attempting to allocate memory for the table, it returns `HM_ERR_ALLOC`.

Alternatively it is possible to populate the fields manually:

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

note that `table` should then be a zero-filled array of length `table_len`.

`DEFAULT_TABLE_LEN` is defined as 10, and `DEFAULT_LOAD_FACTOR` is defined as 0.75.

##### Safe Manipulation Functions

`hash_map_put`, aliased as `put`, takes a pointer to a hash map, a pointer to a key, and a pointer to its associated value. If the key already exists its value is overwritten, else it's created. On success, this function returns `0`. If a memory allocation error occurs, it returns `HM_ERR_ALLOC`.

`hash_map_put_destroy` is the same as `hash_map_put` unless the key already exists in the map, in which case it is deallocated before being overwritten.

`hash_map_get`, aliased as `get`, takes a pointer to a hash map and a pointer to a key. It returns a pointer to the associated value if the key exists. If not, it returns NULL.

`hash_map_drop`, aliased as `drop`, takes a pointer to a hash map and a pointer to a key. It removes the key and its value from the map if present, else returns `HM_W_NOTFOUND`. On success, this function returns `0`. If a memory allocation error occurs, it returns `HM_ERR_ALLOC`.

##### Fast Manipulation Functions

Because there might be cases where the hash function is faster than the equality function and has a very low chance of collisions, there are also functions that use hash equality instead of normal equality. This also saves one hash function call when compared to using something like

    unsigned long eq(void* p1, void* p2)
    {
    	return hash(p1) == hash(p2);
    }

because the hash function is already called once when looking for the appropriate cell.

The functions are `hash_map_fast_put`, `hash_map_fast_put_destroy` `hash_map_fast_get`, and `hash_map_fast_drop`, and have the same return types and arguments as their safe counterparts.

##### Equality and Hashing Functions

A hash function should take a `void*` as its only argument and return a hash of type `unsigned long int`. An equality function should take two `void*`s and return a `bool` (`true` if equal). Default hash and equality functions (`default_hash` and `default_eq`) are provided which work on pointers and thus are only effective when comparing an instance to itself. String equality and hash functions (`string_hash` and `string_eq`) are also provided which work on values.

`hash_map_destroy` takes a pointer to a hash map as an argument and destroys it. This only affects the structure of map itself, not the contained data, since the whole thing works on pointers. If the map was itself created on the heap, it also needs to be freed separately.

### hash_map_io.h

There are also five I/O functions added in hash_map_io.h, separately from the rest because in many cases they won't be needed. In case you do need them, they are as follows:

##### Basic I/O Functions

`hash_map_write` takes a pointer to a stream, a pointer to a hash map, and the sizes of the keys and values. The load factor, element count, and elements are saved to the file. The function does not directly set `ferror`; if it is set, the value corresponds to an error encountered by `fwrite`. The function returns `HM_ERR_STREAM` if it encounters a stream error, `HM_ERR_HEAD` if it encounters an error writing the load factor or size, or one more than the index within the underlying table if it fails while writing an element. On success, it returns 0.

`hash_map_read` takes a pointer to a stream, a pointer to a hash map, and the sizes of the keys and values. The map *must* be initialized before this function is called. The load factor, element count, and elements are loaded from the file. The function does not directly set `ferror`; if it is set, the value corresponds to an error encountered by `fread`. The function returns `HM_ERR_STREAM` if it encounters a stream error, `HM_ERR_HEAD` if it encounters an error writing the load factor or size, or one more than the index within the file if it fails while reading an element. On success, it returns 0.

##### Fast I/O Function

`hash_map_fast_read` is the same as `hash_map_read` except that it uses `hash_map_fast_put` and therefore saves equality checks at the expense of safety.

##### Custom Format I/O Functions

`hash_map_custom_write` takes a pointer to a stream, a pointer to a hash map, and a pointer to a write function. It returns the same values as `hash_map_write`.

`hash_map_custom_read` takes a pointer to a stream, a pointer to a hash map, and a pointer to a read function. It returns the same values as `hash_map_write`.

##### Externally Defined I/O Functions

The function passed to `hash_map_custom_write` should take a stream, a pointer to a key, and a pointer to a value. It should return `1` (the value of `&` on the return values from the two `fwrite()` invocations, assuming nothing else is written) on success and any other value otherwise.

The function passed to `hash_map_custom_read` should take a stream, a pointer to a key, and a pointer to a value. It should return `1` on success and any other value otherwise.
