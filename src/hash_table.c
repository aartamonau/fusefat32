/**
 * @file   hash_table.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Dec  7 23:13:25 2009
 *
 * @brief  Simple hash table implementation.
 *
 *
 */

#include <stdlib.h>

#include "hash_table.h"

/// List used for collision resolution.
/// Each list node stores key value pair.
struct list_t {
  void *key;                    /**< a key */
  void *value;                  /**< a value */

  struct list_t *next;          /**< next node */
};

/**
 * Frees a list.
 *
 * @param list              A list to be freed.
 * @param key_deallocator   Deallocator function for keys.
 * @param value_deallocator Deallocator function for values.
 */
static void
list_free(struct list_t *list,
          deallocator_t key_deallocator,
          deallocator_t value_deallocator);

/**
 * Insert a pair into a list.
 *
 * @param list              List.
 * @param key               Key.
 * @param value             Value.
 * @param equal             Equality function on keys.
 * @param key_cloner        Key cloner.
 * @param value_cloner      Value cloner.
 * @param key_deallocator   Key deallocator.
 * @param value_deallocator Value deallocator.
 *
 * @return New list in case of success. NULL on error.
 */
static struct list_t *
list_insert(struct list_t *list, void *key, void *value,
            equality_function_t equal,
            cloner_t key_cloner, cloner_t value_cloner,
            deallocator_t key_deallocator, deallocator_t value_deallocator);

/**
 * Delete an element from list with specified key.
 *
 * @param list              A list.
 * @param key               A key.
 * @param equal             Equality function on keys.
 * @param key_deallocator   Deallocator for keys.
 * @param value_deallocator Deallocator for values.
 *
 * @return Updated list.
 */
static struct list_t *
list_delete(struct list_t *list, const void *key,
            equality_function_t equal,
            deallocator_t key_deallocator, deallocator_t value_deallocator);

/**
 * Finds an element in the list with the specified key.
 *
 * @param list  A list.
 * @param key   A key.
 * @param equal Equality function on keys.
 *
 * @return A value corresponding to specified key if such value exists. NULL
 *         otherwise.
 */
static void *
list_lookup(struct list_t *list, const void *key,
            equality_function_t equal);


/// Hash table structure;
struct hash_table_t {
  struct list_t **data;        /**< array of lists storing key value pairs */
  ssize_t size;                 /**< a size of array */

  hash_function_t     hash;     /**< hash function */
  equality_function_t equal;    /**< equality checking function */

  cloner_t            key_cloner;     /**< a function to clone keys */
  cloner_t            value_cloner;   /**< a function to clone values */

  deallocator_t       key_deallocator;   /**< a function to deallocate keys */
  deallocator_t       value_deallocator; /**< a function to deallocate values */
};

struct hash_table_t *
hash_table_create(size_t size,
                  hash_function_t     hash,
                  equality_function_t equality,
                  cloner_t            key_cloner,
                  cloner_t            value_cloner,
                  deallocator_t       key_deallocator,
                  deallocator_t       value_deallocator)
{
  struct hash_table_t *hash_table = NULL;
  struct list_t      **data       = NULL;

  hash_table = (struct hash_table_t *) malloc(sizeof(struct hash_table_t));
  if (hash_table == NULL) {
    goto hash_table_create_cleanup;
  }

  hash_table->size              = size;
  hash_table->hash              = hash;
  hash_table->equal             = equality;
  hash_table->key_cloner        = key_cloner;
  hash_table->value_cloner      = value_cloner;
  hash_table->key_deallocator   = key_deallocator;
  hash_table->value_deallocator = value_deallocator;
  hash_table->data              = NULL;

  data = (struct list_t **) malloc(sizeof(struct list_t *) * size);
  if (data == NULL) {
    goto hash_table_create_cleanup;
  }

  for (struct list_t **p = data + size - 1; p >= data; --p) {
    *p = NULL;
  }

  hash_table->data = data;

  return hash_table;

 hash_table_create_cleanup:
  if (hash_table != NULL) {
    if (hash_table->data != NULL) {
      free(hash_table->data);
    }

    free(hash_table);
  }

  /* there is no code for cleaning up allocated lists because error can't
   * occure after those where allocated */

  return NULL;
}

void
hash_table_free(struct hash_table_t *hash_table)
{
  ssize_t size = hash_table->size;

  for (size_t i = 0; i < size; ++i) {
    list_free(hash_table->data[i],
              hash_table->key_deallocator, hash_table->value_deallocator);
  }
}

struct hash_table_t *
hash_table_insert(struct hash_table_t *hash_table,
                  void *key, void *value)
{
  size_t            index = hash_table->hash(key) % hash_table->size;
  struct list_t    *list  = hash_table->data[index];

  list = list_insert(list, key, value,
                     hash_table->equal,
                     hash_table->key_cloner, hash_table->value_cloner,
                     hash_table->key_deallocator,
                     hash_table->value_deallocator);

  if (list != NULL) {
    hash_table->data[index] = list;

    return hash_table;
  } else {
    return NULL;
  }
}

void *
hash_table_lookup(struct hash_table_t *hash_table, const void *key)
{
  size_t            index = hash_table->hash(key) % hash_table->size;
  struct list_t    *list  = hash_table->data[index];

  return list_lookup(list, key, hash_table->equal);
}

void
hash_table_delete(struct hash_table_t *hash_table, const void *key)
{
  size_t            index = hash_table->hash(key) % hash_table->size;
  struct list_t    *list  = hash_table->data[index];

  hash_table->data[index] = list_delete(list, key,
                                        hash_table->equal,
                                        hash_table->key_deallocator,
                                        hash_table->value_deallocator);
}

void
list_free(struct list_t *list,
          deallocator_t key_deallocator,
          deallocator_t value_deallocator)
{
  struct list_t *current   = list;
  bool   deallocate_keys   = key_deallocator != NULL;
  bool   deallocate_values = value_deallocator != NULL;

  while (current != NULL) {
    struct list_t *next = current->next;

    if (deallocate_keys) {
      key_deallocator(current->key);
    }
    if (deallocate_values) {
      value_deallocator(current->value);
    }

    free(current);
    current = next;
  }
}

struct list_t *
list_insert(struct list_t *list, void *key, void *value,
            equality_function_t equal,
            cloner_t key_cloner, cloner_t value_cloner,
            deallocator_t key_deallocator, deallocator_t value_deallocator)
{
  struct list_t *node = (struct list_t *) malloc(sizeof(struct list_t));

  if (node == NULL) {
    goto list_insert_cleanup;
  }

  node->key   = NULL;
  node->value = NULL;

  if (key_cloner != NULL) {
    node->key = key_cloner(key);

    if (node->key == NULL) {
      goto list_insert_cleanup;
    }
  } else {
    node->key = key;
  }

  if (value_cloner != NULL) {
    node->value = value_cloner(value);

    if (node->value == NULL) {
      goto list_insert_cleanup;
    }
  } else {
    node->value = value;
  }

  list = list_delete(list, key, equal, key_deallocator, value_deallocator);

  node->next = list;
  return node;

list_insert_cleanup:
  if (node != NULL) {
    if (node->key != NULL && key_cloner != NULL) {
      key_deallocator(node->key);
    }

    if (node->value != NULL && value_cloner != NULL) {
      value_deallocator(node->value);
    }

    free(node);
  }
  return NULL;
}

struct list_t *
list_delete(struct list_t *list, const void *key,
            equality_function_t equal,
            deallocator_t key_deallocator, deallocator_t value_deallocator)
{
  struct list_t *prev    = NULL;
  struct list_t *current = list;

  while (current != NULL) {
    if (equal(key, current->key)) {
      if (key_deallocator != NULL) {
        key_deallocator(current->key);
      }
      if (value_deallocator != NULL) {
        value_deallocator(current->value);
      }

      struct list_t *next = current->next;
      free(current);

      if (prev != NULL) {
        prev->next = next;
      } else {
        return next;
      }

      return list;
    } else {
      prev    = current;
      current = current->next;
    }
  }

  return list;
}

void *
list_lookup(struct list_t *list, const void *key,
            equality_function_t equal)
{
  while (list != NULL) {
    if (equal(list->key, key)) {
      return list->value;
    } else {
      list = list->next;
    }
  }

  return NULL;
}
