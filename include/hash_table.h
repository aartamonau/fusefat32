/**
 * @file   hash_table.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Dec  7 23:02:14 2009
 *
 * @brief Simple hash table implementation with closed addressing.
 *
 *
 */

#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <stdbool.h>

/// A type of cloner. It takes a pointer to value
/// and returns a new pointer to the newly allocated memory block
/// containing supplied value.
typedef void *(*cloner_t)(const void *);

/// a type of deallocator functions used to free resources
/// allocated for keys and values in hash table
typedef void (*deallocator_t)(void *);

/// a type of hash function
typedef unsigned int (*hash_function_t)(const void *);

/// a type of function to compare to keys for equality
typedef bool (*equality_function_t)(const void *, const void *);

/// incomplete declaration of hash table structure visible from outside
struct hash_table_t;

/* Creates empty hash table using given size as the size for the
   internal storage and a hash function which is used while
   inserting element in a hash table.
   If hash table can't be created then NULL is returned.
*/

/**
 * Creates empty hash table. For each provided cloner corresponding
 * deallocator must be also provided. Though deallocator can be supplied
 * even if corresponding cloner has not been.
 *
 * @param size               A size of internal storage to be used.
 * @param hash               Hash function on keys.
 * @param equality           Equality function on keys.
 * @param key_cloner         Cloner for keys. Can be NULL. In that case a
 *                           pointer to key supplied to appropriate functions
 *                           will be used instead of pointer returned by
 *                           allocator.
 * @param value_cloner       Cloner for values. Can be NULL. Behavior is
 *                           the same as for @em key_allocator parameter.
 * @param key_deallocator    Deallocator for keys.
 * @param value_deallocator  Deallocator for values.
 *
 * @return Created hash table is returned on success. Otherwise NULL is returned
 *         and error is indicated by @em errno variable.
 */
struct hash_table_t *
hash_table_create(size_t size,
                  hash_function_t     hash,
                  equality_function_t equality,
                  cloner_t            key_cloner,
                  cloner_t            value_cloner,
                  deallocator_t       key_deallocator,
                  deallocator_t       value_deallocator);

/**
 * Free all allocated memory held by hash table. If keys' and values'
 * deallocators where provided during hash table construction then those
 * are freed too. Otherwise you must free all the memory held by keys and
 * values manually.
 *
 * @param hash_table hash table to free
 */
void
hash_table_free(struct hash_table_t *hash_table);

/**
 * Adds key value pair to hash table. If a pair with given key already exists
 * in hash table then its value is updated accordingly. Old value is freed only
 * if you provided value_deallocator during hash table creation.
 *
 * @param hash_table Hash table.
 * @param key        An indexing key.
 * @param value      A value corresponding to key.
 *
 * @return A pointer to hash table returned. NULL is returned in case of error,
 *         in which case error is reported using @em errno variable.
 */
struct hash_table_t *
hash_table_insert(struct hash_table_t *hash_table,
                  void *key, void *value);

/**
 * Removes a mapping from hash table corresponding to specified key. Memory
 * held by key and value is deallocated if suitable deallocators where provided.
 *
 * @param hash_table Hash table.
 * @param key        A key.
 */
void
hash_table_delete(struct hash_table_t *hash_table, const void *key);

/**
 * Returns the value mapped to the given key.
 *
 * @param hash_table Hash table.
 * @param key        A key.
 *
 * @return A value corresponding to key if exists. NULL otherwise.
 */
void *
hash_table_lookup(struct hash_table_t *hash_table, const void *key);

#endif
