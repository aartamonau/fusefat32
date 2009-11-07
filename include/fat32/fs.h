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
#include "fat32/errors.h"

/// filesystem device descriptor
struct fat32_fs_t {
  int fd;                       /**< file descriptor of device */

  pthread_mutex_t *write_lock;  /**< Mutex to lock on writing.
                                      Assume the following invariant:
                                      if @em write_lock is not NULL then it's
                                      has been correctly initialized using
                                      @link pthread_mutex_init @endlink
                                */
  
  uint64_t size;                /**< size of underlying block device in bytes */

  struct fat32_bpb_t *bpb;      /**< bios parameters block */
};

/// filesystem parameters
typedef uint32_t params_t;

/** 
 * Opens a device for future work.
 * 
 * @param path a path to the device
 * @param params opening parameters (currently not used)
 * 
 * @return A pointer to created device is returned in case of success.
 *         Otherwise NULL is returned and error is indicated by errno.
 */
enum fat32_error_t
fat32_open_device(const char *path, params_t params,
                  struct fat32_fs_t **device);

/** 
 * Closes device created by the call of @link fat32_open_device @endlink
 * 
 * @param device a device to close
 * 
 * @return As Linus Torvalds noted programmers does not like to check errors
 *         on closing files. Anyway, 0 is returned in case of success and -1
 *         if any error has occured.
 */
int
fat32_close_device(struct fat32_fs_t *device);

#endif
