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
                                            used in #fat32_fs_t */
  uint32_t bytes_per_sector_log;         /**< the number of the highest bit set
                                            in bytes_per_sector */
  const struct fat32_bpb_t *bpb;         /**< a pointer to BPB allocated by and
                                            initialized by
                                            fat32_fs_open() call
                                         */
  const struct fat32_fs_info_t *fs_info; /**< a pointer to FSInfo structure
                                            allocated and initialized by
                                            ::fat32_fs_open call */
};

/// type for each separate entry in FAT
typedef uint32_t fat32_fat_entry_t;

/**
 * Initializes a structure needed to work with file allocation tables.
 * As BPB and FSInfo related functions does not allocate or deallocate memory
 * for the structure itself as all this functions are intended to be used
 * only internally.
 *
 * @param fat a structure to initialize
 * @param fs Partially initialized #fat32_fs_t structure.
 *           By the time of the call #fat32_fs_t::bpb,
 *           fat32_fs_t::fs_info and fat32_fs_t::fd fields must
 *           have been set correctly.
 *
 * @retval FE_OK
 * @retval FE_ERRNO file descriptor of device storing fs can't be dupped
 */
enum fat32_error_t
fat32_fat_init(struct fat32_fat_t *fat,
               const struct fat32_fs_t *fs);

/**
 * Closes all acquired resources for a FAT structure.
 *
 * @param fat a structure to finalize
 *
 * @retval FE_OK
 * @retval FE_ERRNO @em close system call returned an error
 */
enum fat32_error_t
fat32_fat_finalize(struct fat32_fat_t *fat);

/**
 * Returns a FAT entry for a given cluster number
 *
 * @param fat FAT
 * @param cluster Cluster number for which a FAT entry must be returned.
 * @param entry a place to store the resulting entry
 *
 * @retval FE_OK
 * @retval FE_ERRNO @li @em lseek failed
 *                  @li data can't be read from underlying device
 * @retval FE_INVALID_DEV - underlying device file ended prematurely
 */
enum fat32_error_t
fat32_fat_get_entry(const struct fat32_fat_t *fat,
                    uint32_t cluster, fat32_fat_entry_t *entry);

/**
 * Indicates whether cluster corresponding to the given FAT entry is the
 * last in cluster chain.
 *
 * @param entry FAT entry
 *
 * @return boolean value determining whether corresponding cluster is actually
 *         the last
 */
bool
fat32_fat_entry_is_null(fat32_fat_entry_t entry);

/**
 * Indicates whether a cluster corresponding to the given FAT entry is BAD
 * cluster.
 *
 * @param entry FAT entry
 *
 * @return boolean value determining whether a cluster is BAD
 */
bool
fat32_fat_entry_is_bad(fat32_fat_entry_t entry);

/**
 * Transforms a FAT entry to the number of the next cluster in the
 * cluster chain.
 *
 * @param entry FAT entry
 *
 * @return cluster number
 */
uint32_t
fat32_fat_entry_to_cluster(fat32_fat_entry_t entry);

/**
 * Indicates whether the cluster number which is stored in directory entry
 * of some file means that this file is empty.
 *
 * @param cluster a number of cluster to check
 *
 * @return boolean value determining whether cluster is free
 */
bool
fat32_fat_cluster_is_free(uint32_t cluster);

#endif /* _FAT_H_ */
