/**
 * @file   diriter.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Nov  9 17:29:38 2009
 *
 * @brief  Defines an abstraction of iterator for FAT directories.
 *
 *
 */

#ifndef _DIRITER_H_
#define _DIRITER_H_

#include <stdbool.h>
#include <inttypes.h>

#include "fat32/fs.h"
#include "fat32/fs_object.h"
#include "fat32/errors.h"

/// directory iterator structure
struct fat32_diriter_t {
  const struct fat32_fs_t *fs;           /**< file system owning iterated
                                            directory */
  uint32_t                 cluster;      /**< Currently iterated cluster number.
                                            Zero value indicates that there is
                                            nothing to iterate. */
  uint32_t                 offset;       /**< Offset of the next item in cluster
                                            to iterate. */
  bool                     list_dots;    /**< Indicates whether dot and dotdot
                                          * entries must be listed by iterator.
                                          * */
};

/**
 * Creates a directory iterator from #fat32_fs_object_t.
 * Asserts that @em fs_object is directory or root directory.
 *
 * @param fs_object File system object.
 * @param list_dots Specifies whether dot and dotdot entries must be listed.
 *
 * @return Directory iterator. NULL is returned on error and @em errno is set
 *         appropriately.
 */
struct fat32_diriter_t *
fat32_diriter_create(const struct fat32_fs_object_t *fs_object,
                     bool list_dots);

/**
 * Finds next object in the iterator.
 *
 * @param[out] fs_object Next object in iterated sequence.
 * @param      diriter   Iterator.
 *
 * @retval FE_OK
 * @retval FE_ERRNO       IO errors while working with device.
 * @retval FE_INVALID_DEV Device ended prematurely.
 */
enum fat32_error_t
fat32_diriter_next(struct fat32_diriter_t    *diriter,
                   struct fat32_fs_object_t **fs_object);

/**
 * Frees resources hold by directory iterator.
 *
 * @param diriter Directory iterator to free.
 */
void
fat32_diriter_free(struct fat32_diriter_t *diriter);

#endif /* _DIRITER_H_ */
