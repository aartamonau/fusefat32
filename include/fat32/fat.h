/**
 * @file   fat.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sun Nov  8 01:18:33 2009
 * 
 * @brief  Functions and data structures needed to work with file allocation
 *         tables.
 * 
 * 
 */

#ifndef _FAT_H_
#define _FAT_H_

#include "fat32/errors.h"

/// empty fat32_fs_t definition to work around recursive #include 
struct fat32_fs_t;

/// structure encapsulating data needed to work with file allocation tables
struct fat32_fat_t {
  int fd;                                /**< a duplicate of file descriptor
                                            used in @link fat32_fs_t @endlink */
  uint32_t bytes_per_sector_log;         /**< the number of the highest bit set
                                            in bytes_per_sector */
  const struct fat32_bpb_t *bpb;         /**< a pointer to BPB allocated by and
                                            initialized by
                                            @link fat32_fs_open @endlink call
                                         */
  const struct fat32_fs_info_t *fs_info; /**< a pointer to FSInfo structure
                                            allocated and initialized by
                                            @link fat32_fs_open @endlink call */
};

/** 
 * Initializes a structure needed to work with file allocation tables.
 * As BPB and FSInfo related functions does not allocate or deallocate memory
 * for the structure itself as all this functions are intended to be used
 * only internally.
 * 
 * @param fat a structure to initialize
 * @param fs Partially initialized @link fat32_fs_t @endlink structure.
 *           By the time of the call @link fat32_fs_t.bpb @endlink,
 *           @link fat32_fs_t.fs_info @endlink and
 *           @link fat32_fs_t.fs @endlink fields must have been set correctly.
 * 
 * @return operation status
 */
enum fat32_error_t
fat32_fat_init(struct fat32_fat_t *fat,
               const struct fat32_fs_t *fs);

/** 
 * Closes all acquired resources for a FAT structure.
 * 
 * @param fat a structure to finalize
 * 
 * @return operation status
 */
enum fat32_error_t
fat32_fat_finalize(struct fat32_fat_t *fat);


/** 
 * Closes filesystem created by the call of @link fat32_fs_open @endlink
 * 
 * @param fs a file system structure to close
 * 
 * @return status of performed action
 */
enum fat32_error_t
fat32_fs_close(struct fat32_fs_t *fs);

/** 
 * Returns a number of the cluster following the given in FAT.
 * 
 * @param fat FAT
 * @param cluster Value/result parameter. During the call it must contain
 *                interested cluster number. After the return from the call
 *                (if operation was successfull) the following cluster number
 *                is stored here.
 * 
 * @return operation status
 */
enum fat32_error_t
fat32_fat_next_cluster(const struct fat32_fat_t *fat,
		       uint32_t *cluster);

/** 
 * Indicates whether the cluster got by @link fat32_fat_next_cluster @endlink
 * operation actually means the end of linked list of clusters in FAT.
 * 
 * @param cluster a number of cluster to check
 * 
 * @return boolean value determining whether cluster is actually the last
 */
bool
fat32_fat_cluster_is_null(uint32_t cluster);

/** 
 * Indicates whether the cluster number which is stored in directory of some
 * file means that this file is empty.
 * 
 * @param cluster a number of cluster to check
 * 
 * @return boolean value determining whether cluster is free
 */
bool
fat32_fat_cluster_is_free(uint32_t cluster);


#endif /* _FAT_H_ */
