/**
 * @file   fat32/errors.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 17:44:58 2009
 *
 * @brief  Defines errors that can appear while working with filesystem.
 *         Separated to make it usable by the whole program.
 *
 *
 */
#ifndef _FAT32_ERRORS_H_
#define _FAT32_ERRORS_H_

/// Type representing errors which can occur during the work
/// or the driver.
/// Unfortunately to share this type among the code we have to introduce
/// here somewhat specific for some part of code errors too.
enum fat32_error_t {
  FE_OK,                        /**< no errors occured */
  FE_ERRNO,                     /**< indicates that errno specifies error */
  FE_NONBLOCK_DEV,              /**< invalid device is not a block device */
  FE_INVALID_DEV,               /**< invalid device (too small and other
                                   errors of that kind)
                                */
  FE_INVALID_FS,                /**< invalid filesystem */
  FE_INVALID_CLUSTER,           /**< provided cluster number is invalid for
                                   the given file system */
  FE_CLUSTER_CHAIN_ENDED,       /**< cluster chain ended prematurely */
  FE_FS_IS_FULL,                /**< no free space on the file system */
  FE_FS_INCONSISTENT,            /**< returned when because of IO errors file
                                 * system is left in inconsistent state. */
  FE_FS_PARTIALLY_CONSISTENT    /**< file system is in the state in which
                                 * it can be used without visible problems
                                 * but fsck needed to make it strictly
                                 * consistent */
};

#endif /* _FAT32_ERRORS_H_ */
