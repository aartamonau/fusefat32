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
