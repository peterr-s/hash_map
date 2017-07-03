#include "hash_map.h"

/* initialize a hash map structure */
short int hash_map_init(hash_map* map, unsigned long int(* hash_fn)(void* key), bool(* eq_fn)(void* p1, void* p2), unsigned long int start_len, float load_factor)
{
	map->table = calloc(start_len, sizeof(node*));
	if(!map->table)
		return HM_ERR_ALLOC;
	map->hash_fn = hash_fn;
	map->eq_fn = eq_fn;
	map->table_len = start_len;
	map->load_factor = load_factor;
	map->element_ct = 0;
	
	return 0;
}

/* add a key-value pair to a hash map, resizing and rehashing if necessary */
short int hash_map_put(hash_map* map, void* key, void* value)
{
	node* n_node,
		* s_node;
	unsigned long int node_idx;
	
	/* perform resize and rehash if necessary */
	if((++ map->element_ct) / map->table_len > map->load_factor)
	{
		size_t n_len = map->table_len << 1;
		node** temp = calloc(n_len, sizeof(node*));
		if(!temp)
			return HM_ERR_ALLOC;
		
		/* for each element in the table */
		for(i = 0; i < map->table_len; i ++)
		{
			/* traverse down the linked list */
			node* current,
				* next;
			
			/* guard against empty elements */
			current = map->table[i];
			while(current)
			{
				unsigned long int npos;
				
				/* prepare lookahead pointer */
				next = current->next;
				
				/* rehash and copy each item */
				npos = (map->hash_fn(current->key)) % n_len;
				current->next = temp[npos];
				temp[npos] = current;
				
				/* advance to next list element */
				current = next;
			}
		}
		
		free(map->table);
		map->table = temp;
		map->table_len = n_len;
	}
	
	/* check whether item is already in map */
	node_idx = (*map->hash_fn)(key) % map->table_len;
	s_node = map->table[node_idx];
	while(s_node)
	{
		if((map->eq_fn)(s_node->key, key))
		{
			/* update value and return if found */
			s_node->value = value;
			return 0;
		}
		s_node = s_node->next;
	}
	
	/* create a new node to hold data */
	n_node = malloc(sizeof(node));
	if(!n_node)
		return HM_ERR_ALLOC;
	n_node->key = key;
	n_node->value = value;
	n_node->next = map->table[node_idx];
	
	/* add new node to table */
	map->table[node_idx] = n_node;
	
	return 0;
}

/* same as above, but replaces equality check with hash equality check */
short int hash_map_fast_put(hash_map* map, void* key, void* value)
{
	node* n_node,
		* s_node;
	unsigned long int node_idx,
		node_hash;
	
	if((++ map->element_ct) / map->table_len > map->load_factor)
	{
		size_t n_len = map->table_len << 1;
		node** temp = calloc(n_len, sizeof(node*));
		if(!temp)
			return HM_ERR_ALLOC;
		
		/* for each element in the table */
		for(i = 0; i < map->table_len; i ++)
		{
			/* traverse down the linked list */
			node* current,
				* next;
			
			/* guard against empty elements */
			current = map->table[i];
			while(current)
			{
				unsigned long int npos;
				
				/* prepare lookahead pointer */
				next = current->next;
				
				/* rehash and copy each item */
				npos = (map->hash_fn(current->key)) % n_len;
				current->next = temp[npos];
				temp[npos] = current;
				
				/* advance to next list element */
				current = next;
			}
		}
		
		free(map->table);
		map->table = temp;
		map->table_len = n_len;
	}
	
	/* check whether item is already in map */
	node_hash = (*map->hash_fn)(key);
	node_idx = node_hash % map->table_len;
	s_node = map->table[node_idx];
	while(s_node)
	{
		if(node_hash == (map->hash_fn)(s_node->key))
		{
			/* update value and return if found */
			s_node->value = value;
			return 0;
		}
		s_node = s_node->next;
	}
	
	/* create a new node to hold data */
	n_node = malloc(sizeof(node));
	if(!n_node)
		return HM_ERR_ALLOC;
	n_node->key = key;
	n_node->value = value;
	n_node->next = map->table[node_idx];
	
	/* add new node to table */
	map->table[node_idx] = n_node;
}

/* get the value associated with a key
 * returns NULL if key does not exist in map
 */
void* hash_map_get(hash_map* map, void* key)
{
	node* current;
	
	current = map->table[(map->hash_fn)(key) % map->table_len];
	while(current)
	{
		if((map->eq_fn)(key, current->key))
			return current->value;
		
		current = current->next;
	}
	
	return NULL;
}

/* same as above, but replaces equality check with hash equality check */
void* hash_map_fast_get(hash_map* map, void* key)
{
	node* current;
	unsigned long int k_hash;
	
	k_hash = (map->hash_fn)(key);
	current = map->table[k_hash % map->table_len];
	while(current)
	{
		if(k_hash == (map->hash_fn)(current->key))
			return current->value;
		
		current = current->next;
	}
	
	return NULL;
}

/* removes an item from the hashmap if present (else no-op) */
short int hash_map_drop(hash_map* map, void* key)
{
	node* current,
		* parent = NULL;
	unsigned long int idx = (map->hash_fn)(key) % map->table_len;
	
	current = map->table[idx];
	while(current)
	{
		if((map->eq_fn)(key, current->key))
		{
			/* redirect the node chain around it, then destroy */
			if(parent)
				parent->next = current->next;
			else
				map->table[idx] = current->next;
			free(current); /* nodes are always malloc'ed */
			
			/* perform resize and rehash if necessary */
			if((map->table_len > 10) && (-- map->element_ct) / (map->table_len << 1) < map->load_factor)
			{
				size_t n_len = map->table_len >> 1;
				node** temp = calloc(n_len, sizeof(node*));
				if(!temp)
					return HM_ERR_ALLOC;
				
				/* for each element in the table */
				for(i = 0; i < map->table_len; i ++)
				{
					/* traverse down the linked list */
					node* current,
						* next;
					
					/* guard against empty elements */
					current = map->table[i];
					while(current)
					{
						unsigned long int npos;
						
						/* prepare lookahead pointer */
						next = current->next;
						
						/* rehash and copy each item */
						npos = (map->hash_fn(current->key)) % n_len;
						current->next = temp[npos];
						temp[npos] = current;
						
						/* advance to next list element */
						current = next;
					}
				}
				
				free(map->table);
				map->table = temp;
				map->table_len = n_len;
			}
			
			/* stop looking since node was found */
			return 0;
		}
		
		parent = current;
		current = current->next;
	}
	
	return HM_W_NOTFOUND;
}

/* same as above, but replaces equality check with hash equality check */
short int hash_map_fast_drop(hash_map* map, void* key)
{
	node* current,
		* parent = NULL;
	unsigned long int k_hash,
		idx;
	
	k_hash = (map->hash_fn)(key);
	idx = k_hash % map->table_len;
	
	current = map->table[idx];
	while(current)
	{
		if(k_hash == (map->hash_fn)(current->key))
		{
			/* redirect the node chain around it, then destroy */
			if(parent)
				parent->next = current->next;
			else
				map->table[idx] = current->next;
			free(current); /* nodes are always malloc'ed */
			
			/* perform resize and rehash if necessary */
			if((map->table_len > 10) && (-- map->element_ct) / (map->table_len << 1) < map->load_factor)
			{
				size_t n_len = map->table_len >> 1;
				node** temp = calloc(n_len, sizeof(node*));
				if(!temp)
					return HM_ERR_ALLOC;
				
				/* for each element in the table */
				for(i = 0; i < map->table_len; i ++)
				{
					/* traverse down the linked list */
					node* current,
						* next;
					
					/* guard against empty elements */
					current = map->table[i];
					while(current)
					{
						unsigned long int npos;
						
						/* prepare lookahead pointer */
						next = current->next;
						
						/* rehash and copy each item */
						npos = (map->hash_fn(current->key)) % n_len;
						current->next = temp[npos];
						temp[npos] = current;
						
						/* advance to next list element */
						current = next;
					}
				}
				
				free(map->table);
				map->table = temp;
				map->table_len = n_len;
			}
			
			/* stop looking since node was found */
			return 0;
		}
		
		parent = current;
		current = current->next;
	}
	
	return HM_W_NOTFOUND;
}

/* destroys a hashmap (does not touch pointed data) */
void hash_map_destroy(hash_map* map)
{
	node* current,
		* next;
	
	/* goes down each list, deleting all nodes */
	for(i = 0; i < map->table_len; i++)
	{
		current = map->table[i];
		while(current)
		{
			next = current->next;
			free(current);
			current = next;
		}
	}
	
	/* deletes the table */
	free(map->table);
}

/* simple "hash" function (uses pointer as hash code) */
unsigned long int default_hash(void* key)
{
	return (unsigned long int) key;
}

/* simple string hash function */
unsigned long int string_hash(void* key)
{
	unsigned long int hash = 0;
	char* string = key;
	
	while(*string)
	{
		hash += *string; /* addition followed by multiplication makes collisions of scrambled strings with the same characters less likely */
		hash *= 7;
		string ++;
	}
	
	return hash;
}

/* simple equality checker (compares pointers) */
bool default_eq(void* p1, void* p2)
{
	return p1 == p2;
}

/* string equality checker (uses strcmp) */
bool string_eq(void* p1, void* p2)
{
	return !strcmp((char*)p1, (char*)p2);
}