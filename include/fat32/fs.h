/**
 * @file   fs.h
 * @author  <aliaksiej.artamonau@gmail.com>
 * @date   Tue Sep 29 20:35:58 2009
 *
 * @brief  Declares common function for working with filesystem.
 *
 */


#ifndef _FS_H_
#define _FS_H_

#include <stdbool.h>

#include <inttypes.h>
#include <sys/stat.h>
#include <pthread.h>

#include "hash_table.h"

#include "fat32/bpb.h"
#include "fat32/fs_info.h"
#include "fat32/fat.h"
#include "fat32/errors.h"

/// empty fat32_fs_object_t definition to work around recursive #include
struct fat32_fs_object_t;

/// filesystem descriptor
struct fat32_fs_t {
  int              fd;             /**< file descriptor of device where
                                      filesystem is stored */

  pthread_mutex_t *write_lock;     /**< Mutex to lock on writing.
                                        Assume the following invariant:
                                        if @em write_lock is not NULL then it
                                        has been correctly initialized using
                                        @em pthread_mutex_init
                                   */

  uint64_t size;                   /**< size of underlying block device in
                                      bytes */

  struct fat32_bpb_t     *bpb;     /**< bios parameters block */
  struct fat32_fs_info_t *fs_info; /**< FSInfo */
  struct fat32_fat_t     *fat;     /**< FAT-related data */

  struct hash_table_t    *file_table; /**< hash table containg information
                                       * about open files */
  struct hash_table_t    *fh_table; /**< hash table containing open
                                       file handles and corresponding to them
                                       fs objects */
  struct fat32_fh_allocator_t *fh_allocator; /**< allocator for file handles */

  uint32_t cluster_size;            /**< cached size of the cluster on file
                                     * system */
};

/// filesystem parameters
struct fat32_fs_params_t {
  size_t file_table_size;     /**< a size of hash table for open files */
  size_t fh_table_size;       /**< a size of hash table for file handles  */
};

/**
 * Opens a filesystem for future work.
 *
 * @param      path    a path to the device containing filestystem
 * @param      params  opening parameters (currently not used)
 * @param      fs      a pointer where a pointer to resulting file system
 *                     structure will be stored
 * @retval FE_OK
 * @retval FE_ERRNO @li memory allocation error
 *                  @li unable to create or initialize synchronization objects
 *                  @li unable to read/lseek/etc on device file
 * @retval FE_NONBLOCK_DEV device is not a block device
 * @retval FE_INVALID_DEV device ends prematurely
 * @retval FE_INVALID_FS BPB/FSInfo on the device is inconsistent
 */
enum fat32_error_t
fat32_fs_open(const char *path, const struct fat32_fs_params_t *params,
        struct fat32_fs_t **fs);

/**
 * Closes filesystem created by the call of ::fat32_fs_open
 *
 * @param fs a file system structure to close
 *
 * @retval FE_OK
 * @retval FE_ERRNO close operation returned an error
 */
enum fat32_error_t
fat32_fs_close(struct fat32_fs_t *fs);

/**
 * Reads a cluster into the buffer. Function does not restore file offset
 * after the call.
 *
 * @param fs file system object
 * @param buffer which size is greater or equal to
 *               #fat32_bpb_t::bytes_per_sector *
 *               #fat32_bpb_t.sectors_per_cluster
 * @param cluster a number of cluster to read
 *
 *
 * @retval FE_OK
 * @retval FE_ERRNO
 * @retval FE_INVALID_CLUSTER Invalid cluster number specified. This can be
 *                            either programming error or inconsistency on
 *                            the file system.
 * @retval FE_INVALID_DEV     Data ends prematurely.
 */
enum fat32_error_t
fat32_fs_read_cluster(const struct fat32_fs_t *fs, void *buffer,
                      uint32_t cluster);

/**
 * Returns a fs object specified by its path.
 *
 * @param      fs        File system.
 * @param      path      A path to object.
 * @param[out] fs_object File system object stored here on success. NULL is
 *                       stored if object with given path can't be found.
 * @param[out] parent    If this parameter is not NULL then a parent of
 *                       fs_object is stored here. Even if wrong path is
 *                       specified then the last directory checked is stored
 *                       in this parameter.
 *                       NULL is returned as a parent for root directory.
 *
 * @retval FE_OK
 * @retval FE_ERRNO       IO errors while working with device.
 * @retval FE_INVALID_DEV Device ended prematurely.
 */
enum fat32_error_t
fat32_fs_get_object(const struct fat32_fs_t *fs,
                    const char *path,
                    struct fat32_fs_object_t **fs_object,
                    struct fat32_fs_object_t **parent);
#endif
