/**
 * @file   fs_info.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 16:55:24 2009
 *
 * @brief  FSInfo structures and related function's prototypes.
 *
 * @todo keep correct free space hint
 * @todo keep correct free cluster hint
 */
#ifndef _FS_INFO_H_
#define _FS_INFO_H_

#include <stdbool.h>
#include <inttypes.h>

#include "fat32/bpb.h"
#include "fat32/errors.h"

/// number of bytes in the first reserved block in fsinfo structure
#define FS_INFO_RESERVED_BLOCK1_SIZE 480

/// number of bytes in the second reserved block of fsinfo structure
#define FS_INFO_RESERVED_BLOCK2_SIZE 12

/// Structure describing FSInfo sector
struct fat32_fs_info_t {
  uint32_t lead_signature;  /**< lead signature used to validate fsinfo
                               sector */
  uint8_t  reserved1[FS_INFO_RESERVED_BLOCK1_SIZE]; /**< reserved for
                                                       future expansion */
  uint32_t struct_signature;    /**< yet another validating signature */
  uint32_t last_free_count;   /**< last known free cluster count
                                   (0xffffffff means that it's unknown) */
  uint32_t free_cluster_hint;   /**< a hint showing from which cluster we
                                   should search for free one
                                   (0xffffffff means that there is no hint) */
  uint8_t  reserved2[FS_INFO_RESERVED_BLOCK2_SIZE]; /**< reserved for future
                                                       expansion */
  uint32_t trail_signature;     /**< used for validation */

} __attribute__((packed));

/**
 * Checks where read fsinfo structure is correct
 *
 * @param fs_info a structure to check
 *
 * @return @em true if correct. @em false otherwise.
 */
bool
fat32_fs_info_check_validity(const struct fat32_fs_info_t *fs_info);

/**
 * Logs verbose information about FSInfo sector.
 *
 * @param fs_info FSInfo structure
 *
 * @return Upon success zero is returned. Otherwise negative value is
 * returned and error is indicated in errno.
 */
int
fat32_fs_info_verbose_info(const struct fat32_fs_info_t *fs_info);

/**
 * Read fs_info structure from file. An offset from which reading occurs
 * is taken from BPB. This functions restores original file position after
 * reading all needed data.
 *
 * @param fd a file to read FSInfo from
 * @param bpb BPB structure
 * @param fs_info a place to store readed data
 *
 * @retval FE_OK
 * @retval FE_ERRNO @li unable to @em lseek in the underlying device file
 *                  @li unable to @em read from the underlying device file
 * @retval FE_INVALID_DEV device file ended prematurely
 * @retval FE_INVALID_FS FSInfo structure read is inconsistent
 */
enum fat32_error_t
fat32_fs_info_read(int fd, const struct fat32_bpb_t *bpb,
       struct fat32_fs_info_t *fs_info);

#endif /* _FS_INFO_H_ */
