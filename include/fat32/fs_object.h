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

#define REIMPORT_INLINES
#include "fat32/fs.h"
#undef REIMPORT_INLINES

#include "utils/inlines.h"

/// emtpy fat32_direntry_t definition to work around inlining issues.
struct fat32_direntry_t;

/// possible types of file system objects
enum fat32_fs_object_type_t {
  FAT32_FS_OBJECT_FILE,         /**< ordinary file */
  FAT32_FS_OBJECT_DIR,          /**< ordinary directory */
  FAT32_FS_OBJECT_ROOT_DIR      /**< root directory */
};

/// structure defining file system object
/// @todo extend
struct fat32_fs_object_t {
  enum fat32_fs_object_type_t type; /**< type of underlying file system
                                       object */
  char                       *name; /**< utf-8 encoded name of the object
                                       (is null for root directory) */
  struct fat32_direntry_t    *direntry; /**< direntry corresponding to the
                                           object (is null for root
                                           directory) */
  const struct fat32_fs_t    *fs;   /**< file system containing the object */
};

/**
 * Creates a file system object for the root directory of fs. After the usage
 * the object must be manually freed by ::fat32_fs_object_free function call.
 *
 * @param fs File system object. Must not be freed while fs object is in use.
 *
 * @return File system object is returned in case of success. In case
 *         of errors @em NULL is returned and @em errno is set appropriately.
 */
struct fat32_fs_object_t *
fat32_fs_object_root_dir(const struct fat32_fs_t *fs);

/**
 * Creates a file system object for the directory entry. After the usage
 * the object must be manually freed by ::fat32_fs_object_free function call.
 *
 * @param fs File system object. Must not be freed while fs object is in use.
 * @param direntry Directory entry. It's copied to fs_object so it can be safely
 *                 freed while fs object is still in use.
 * @param name A name of fs object. Generally it's impossible to determine the
 *             name only from directory entry that's why it's picked to
 *             parameters list.
 *
 * @return File system object is returned in case of success. In case
 *         of errors @em NULL is returned and @em errno is set appropriately.
 */
struct fat32_fs_object_t *
fat32_fs_object_direntry(const struct fat32_fs_t       *fs,
                         const struct fat32_direntry_t *direntry,
                         const char *name);

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

/**
 * Determines whether file system object is an ordinary file.
 *
 * @param fs_object File system object.
 *
 * @return Boolean result.
 */
INLINE bool
fat32_fs_object_is_file(const struct fat32_fs_object_t *fs_object)
{
  return fs_object->type == FAT32_FS_OBJECT_FILE;
}

/**
 * Determines whether file system object is a directory.
 *
 * @param fs_object File system object
 *
 * @return Boolean result.
 */
INLINE bool
fat32_fs_object_is_directory(const struct fat32_fs_object_t *fs_object)
{
  return !fat32_fs_object_is_file(fs_object);
}

/**
 * A cloner for fs objects to use with hash tables.
 *
 * @param fs_object A pointer to fs_object.
 *
 * @return New fs object of NULL on error.
 */
void *
fat32_fs_object_cloner(const void *fs_object);

#endif /* _FS_OBJECT_H_ */
