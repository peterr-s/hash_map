#include "hash_map_io.h"

/* write a hash map to a file
 * does not save the hashing or equality functions, nor does it save the actual positions of the elements within the table
 * returns HM_ERR_STREAM if the file is not correctly opened
 * returns HM_ERR_IO_HEAD if an error is encountered while writing the load factor or element count
 * returns 1 more than the index within the table if an error is encountered while writing data
 * returns 0 if hash map is written successfully
 */
int hash_map_write(FILE* stream, hash_map* map, size_t key_sz, size_t value_sz)
{
	unsigned long int i;
	
	/* check if file is properly open */
	if(!stream)
		return HM_ERR_STREAM;
	if(ferror(stream))
		return HM_ERR_STREAM;
	if(feof(stream))
		return HM_ERR_STREAM;
	
	/* write load factor */
	if(fwrite(&(map->load_factor), sizeof(float), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* write number of elements */
	if(fwrite(&(map->element_ct), sizeof(unsigned long int), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* write each key followed by its value */
	for(i = 0; i < map->table_len; i ++)
	{
		node* current = map->table[i];
		while(current)
		{
			int key_return_value = fwrite(current->key, key_sz, 1, stream); /* this is used to force a sequence point so that the key is written before the value */
			
			/* ensure that both the key and its value are written successfully */
			if((key_return_value & fwrite(current->value, value_sz, 1, stream)) != 1)
				return i + 1;
			
			current = current->next;
		}
	}
	
	return i;
}

/* read a hash map from a file
 * does not load the hashing or equality functions
 * requires that the map be initialized with valid hashing and equality functions
 * returns HM_ERR_STREAM if the file is not correctly opened
 * returns HM_ERR_HEAD if an error is encountered while reading the head data
 * returns 1 more than the index within the file if an error is encountered while reading data
 * returns 0 if hash map is read successfully
 */
int hash_map_read(FILE* stream, hash_map* map, size_t key_sz, size_t value_sz, unsigned short int flags)
{
	unsigned long int i,
		element_ct;

	/* check if file is properly open */
	if(!stream)
		return HM_ERR_STREAM;
	if(ferror(stream))
		return HM_ERR_STREAM;
	if(feof(stream))
		return HM_ERR_STREAM;
	
	/* read load factor */
	if(fread(&(map->load_factor), sizeof(float), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* read number of elements */
	if(fread(&element_ct, sizeof(unsigned long int), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* read each key followed by its value */
	for(i = 0; i < element_ct; i ++)
	{
		void* key = malloc(key_sz);
		void* value = malloc(value_sz);
		
		int key_return_value = fread(key, key_sz, 1, stream); /* this is used to force a sequence point */
		
		if((key_return_value & fread(value, value_sz, 1, stream)) != 1)
			return i;
		
		if(hash_map_put(map, key, value, flags))
			return i;
	}
	return i;
}

int hash_map_custom_write(FILE* stream, hash_map* map, int(* write_fn)(FILE* stream, void* key, void* value))
{
	unsigned long int i;
	
	/* check if file is properly open */
	if(!stream)
		return HM_ERR_STREAM;
	if(ferror(stream))
		return HM_ERR_STREAM;
	if(feof(stream))
		return HM_ERR_STREAM;
	
	/* write load factor */
	if(fwrite(&(map->load_factor), sizeof(float), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* write number of elements */
	if(fwrite(&(map->element_ct), sizeof(unsigned long int), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* write each key followed by its value */
	for(i = 0; i < map->table_len; i ++)
	{
		node* current = map->table[i];
		while(current)
		{
			/* ensure that both the key and its value are written successfully */
			if(write_fn(stream, current->key, current->value) != 1)
				return i + 1;
			
			current = current->next;
		}
	}
	
	return i;
}

int hash_map_custom_read(FILE* stream, hash_map* map, int(* read_fn)(FILE* stream, void** key, void** value), unsigned short int flags)
{
	unsigned long int i,
		element_ct;

	/* check if file is properly open */
	if(!stream)
		return HM_ERR_STREAM;
	if(ferror(stream))
		return HM_ERR_STREAM;
	if(feof(stream))
		return HM_ERR_STREAM;
	
	/* read load factor */
	if(fread(&(map->load_factor), sizeof(float), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* read number of elements */
	if(fread(&element_ct, sizeof(unsigned long int), 1, stream) != 1)
		return HM_ERR_IO_HEAD;
	
	/* read each key followed by its value */
	for(i = 0; i < element_ct; i ++)
	{
		void** key = malloc(sizeof(void*));
		void** value = malloc(sizeof(void*));
		
		if(read_fn(stream, key, value) != 1)
			return i;
		
		if(hash_map_put(map, *key, *value, flags))
			return i;

		free(key);
		free(value);
	}
	
	return i;
}
