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

#include <inttypes.h>

#include "fat32/fs.h"
#include "fat32/fs_object.h"
#include "fat32/errors.h"

/// directory iterator structure
struct fat32_diriter_t {
  const struct fat32_fs_t *fs; /**< file system owning iterated directory */
  uint32_t                 cluster; /**< currently iterated cluster number */
  uint32_t                 offset;  /**< offset of the next item in cluster to
				       iterate */
  void                    *buffer;  /**< buffer storing currently iterade
				       cluster data */
};

struct fat32_diriter_t *
fat32_diriter_create(const struct fat32_fs_object_t *fs_object);

#endif /* _DIRITER_H_ */
