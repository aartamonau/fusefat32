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

#include "fat32/bpb.h"
#include "fat32/fs_info.h"
#include "fat32/fat.h"
#include "fat32/errors.h"

/// filesystem descriptor
struct fat32_fs_t {
  int              fd;             /**< file descriptor of device where
				      filesystem is stored */

  pthread_mutex_t *write_lock;     /**< Mutex to lock on writing.
                                        Assume the following invariant:
                                        if @em write_lock is not NULL then it
                                        has been correctly initialized using
                                        @link pthread_mutex_init @endlink
                                   */
  
  uint64_t size;                   /**< size of underlying block device in
				      bytes */

  struct fat32_bpb_t     *bpb;     /**< bios parameters block */
  struct fat32_fs_info_t *fs_info; /**< FSInfo */
  struct fat32_fat_t     *fat;	   /**< FAT-related data */
};

/// filesystem parameters
typedef uint32_t params_t;

/** 
 * Opens a filesystem for future work.
 * 
 * @param path   a path to the device containing filestystem
 * @param params opening parameters (currently not used)
 * @param fs     a pointer where a pointer to resulting file system structure
 *               will be stored
 * 
 * @return status of performed operation
 */
enum fat32_error_t
fat32_fs_open(const char *path, params_t params,
	      struct fat32_fs_t **fs);

#endif
