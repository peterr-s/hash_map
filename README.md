### hash_map.h

I couldn't find a general-purpose cross-platform C hash map implementation and needed one for a project I'm working on, so I wrote one. If it can be of use to you, have at it.

Everything is absolutely standards compliant C89 with no assumptions whatsoever about undefined behavior.

#### Usage

This file defines a `hash_map` structure and basic methods to manipulate it. The hash map works entirely with void pointers, so it does not create additional copies of data that is added to it or destroy elements that are removed.

`hash_map_init` takes a pointer to a hash map, a pointer to a hash function, a pointer to an equality function, a start size for the internal table, and a load factor. Alternatively it is possible to populate the fields manually:

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

`hash_map_add`, aliased as `add`, takes a pointer to a hash map, a pointer to a key, and a pointer to its associated value. If the key already exists its value is overwritten, else it's created.

`hash_map_get`, aliased as `get`, takes a pointer to a hash map and a pointer to a key. It returns a pointer to the associated value if the key exists. If not, it returns NULL.

`hash_map_drop`, aliased as `drop`, takes a pointer to a hash map and a pointer to a key. It removes the key and its value from the map if present, else is a no-op.

##### Fast Manipulation Functions

Because there might be cases where the hash function is faster than the equality function and has a very low chance of collisions, there are also functions that use hash equality instead of normal equality. This also saves one hash function call when compared to using something like

    unsigned long eq(void* p1, void* p2)
    {
    	return hash(p1) == hash(p2);
    }

because the hash function is already called once when looking for the appropriate cell.

The functions are `hash_map_fast_add`, `hash_map_fast_get`, and `hash_map_fast_drop`, and have the same return types and arguments as their safe counterparts.

##### Equality and Hashing Functions

A hash function should take a `void*` as its only argument and return a hash of type `unsigned long int`. An equality function should take two `void*`s and return a `bool` (`true` if equal). Default hash and equality functions (`default_hash` and `default_eq`) are provided which work on pointers and thus are only effective when comparing an instance to itself. String equality and hash functions (`string_hash` and `string_eq`) are also provided which work on values.

`hash_map_destroy` takes a hash map as an argument and destroys it. This only affects the map itself, not the contained data, since the whole thing works on pointers.
