/**
 * @file   fh.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Dec  7 17:36:26 2009
 *
 * @brief  File handles' related functionality.
 *
 *
 */
#ifndef _FH_TABLE_H_
#define _FH_TABLE_H_

#include <stdbool.h>
#include <inttypes.h>

/// a type of file handle
typedef uint64_t fat32_fh_t;

/// Type representing file handles allocator.
/// Oversimplified implementation. TODO: improve.
struct fat32_fh_allocator_t {
  fat32_fh_t last_fh;              /**< last allocated file handle */
};

/**
 * Creates file handles allocator.
 *
 *
 * @return New allocator. NULL on error. Error is specified using @em errno.
 */
struct fat32_fh_allocator_t *
fat32_fh_allocator_create(void);

/**
 * Frees memory held by file handles allocator.
 *
 * @param allocator Allocator to free.
 */
void
fat32_fh_allocator_free(struct fat32_fh_allocator_t *allocator);

/**
 * Allocates new file handle.
 *
 * @param      allocator Allocator used to allocate handlers.
 * @param[out] fh        New file handle stored here.
 *
 * @retval     true      Handle allocated successfully.
 * @retval     false     It's impossible to allocate new handlers.
 */
bool
fat32_fh_allocate(struct fat32_fh_allocator_t *allocator,
                  fat32_fh_t *fh);

/**
 * Hash function on file handles.
 *
 * @param fh file handle
 *
 * @return handle's hash
 */
unsigned int
fat32_fh_hash(const void *fh);

/**
 * Function that checks equality of file handles.
 *
 * @param fh_a first file handle
 * @param fh_b second file handle
 *
 * @return boolean value indicating whether handles are equal
 */
bool
fat32_fh_equal(const void *fh_a, const void *fh_b);

/**
 * A cloner function to be used with hash tables.
 *
 * @param fh file handle
 *
 * @return allocated hash table or NULL in case of error
 */
void *
fat32_fh_cloner(const void *fh);

#endif /* _FH_TABLE_H_ */
