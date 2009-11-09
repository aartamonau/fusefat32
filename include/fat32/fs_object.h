/**
 * @file   fs_object.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Nov  9 18:07:18 2009
 * 
 * @brief  High level view on the object filesystem stores.
 * 
 * 
 */

#ifndef _FS_OBJECT_H_
#define _FS_OBJECT_H_

#include <inttypes.h>

#include "fat32/fs.h"
#include "fat32/direntry.h"

/// possible types of file system objects
enum fat32_fs_object_type_t {
  FAT32_FS_OBJECT_FILE,         /**< ordinary file */
  FAT32_FS_OBJECT_DIR,          /**< ordinary directory */
  FAT32_FS_OBJECT_ROOT_DIR      /**< root directory */
};

/// structure defining file system object
/// @todo extend
struct fat32_fs_object_t {
  enum fat32_fs_object_type_t    type; /**< type of underlying file system
                                         object */
  char                          *name; /**< utf-8 encoded name of the object
					 (is null for root directory) */
  const struct fat32_direntry_t *direntry; /**< direntry corresponding to the
                                             object (is null for root
                                             directory) */
  const struct fat32_fs_t       *fs;   /**< file system containing the object */
};

/** 
 * Creates a file system object for the root directory of fs. After the usage
 * the object must be manually freed by ::fat32_fs_object_free function call.
 * 
 * @param fs filesystem
 * 
 * @return File system object is returned in case of success. In case
 *         of errors @em NULL is returned and @em errno is set appropriately.
 */
struct fat32_fs_object_t *
fat32_fs_object_root_dir(const struct fat32_fs_t *fs);

/** 
 * Deallocates all memory held by file system object.
 * 
 * @param fs_object an object to free
 */
void
fat32_fs_object_free(struct fat32_fs_object_t *fs_object);

/** 
 * Returns a number of the first cluster of file system object.
 * 
 * @param fs_object file system object
 * 
 * @return cluster number
 */
uint32_t
fat32_fs_object_first_cluster(const struct fat32_fs_object_t *fs_object);

#endif /* _FS_OBJECT_H_ */
