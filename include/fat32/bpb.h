/**
 * @file   bpb.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Wed Sep  9 12:25:17 2009
 * 
 * @brief  Defines data structures describing FAT32 boot sector and
 * bios parameters block.
 * 
 */
#ifndef _BPB_H_
#define _BPB_H_

#include <stdio.h>
#include <stdbool.h>

#include <inttypes.h>

#include "fat32/errors.h"

/// maximum size of cluster in bytes
extern const uint16_t MAX_CLUSTER_SIZE;

/// value of @link fat32_bpb_t.fs_version @endlink field which specifies
/// that given filesystem is FAT32
extern const uint16_t FAT32_FS_VERSION;

/// Structure describing all available BPB parameters
struct fat32_bpb_t {
  uint8_t  jmp_boot[3];            /**< jump instruction to boot code */
  uint8_t  oem_name[8];            /**< OEM name */
  uint16_t bytes_per_sector;       /**< number of bytes in each sector */
  uint8_t  sectors_per_cluster;    /**< number of sectors in cluster */
  uint16_t reserved_sectors_count; /**< number of reserverd sectors */
  uint8_t  fats_count;             /**< nubmer of fats */
  uint16_t root_entries_count;     /**< number of root entries. For FAT32 must
                                      be set to 0. */
  uint16_t total_sectors_count_16; /**< 16-bit total sectors count. For FAT32
                                      must be to 0.*/
  uint8_t  media_type;             /**<  type of media. */
  uint16_t fat_size_16;            /**<  16-bit size of FAT. Must be set to
                                      0 on FAT32. */
  uint16_t sectors_per_track;      /**<  number of sectors per track */
  uint16_t heads_number;           /**<  number of heads */
  uint32_t hidden_sectors_count;   /**<  number of hidden sectors */
  uint32_t total_sectors_count;    /**<  32-bit total sectors count*/

  /* fat32-specific fields  */
  uint32_t fat_size;               /**<  the size of one FAT */
  uint16_t extended_flags;         /**<  extended flags. @todo macros */

  uint16_t fs_version;             /**<  0x0000 for FAT32 */
  uint32_t root_cluster;           /**<  a number of the first cluster of root
                                      directory */
  uint16_t fs_info_sector;         /**<  sector number of FSINFO in reserved
                                      area of FAT32 */
  uint16_t backup_boot_sector;     /**<  if non-zero indicates a sector number
                                      in reserved area with boot record
                                      backup */
  uint8_t  reserved[8];            /**<  reserved for future expansion */
  uint8_t  drive_number;           /**<  int 0x13 drive number. OS specific. */
  uint8_t  nt_reserved;            /**<  used by Windows NT. Must be set to
                                      0 */
  uint8_t  boot_signature;         /**<  extended boot signature (0x29). If set
                                      indicates that the following three fields
                                      are present. */
  uint32_t volume_id;              /**<  volume serial number */
  uint8_t  volume_label[11];       /**<  matches 11-byte volume label recorded
                                      in root directory */
  uint8_t  fs_type[8];             /**<  filesystem type */
} __attribute__((packed));

/** 
 * Prints out verbose information about BPB to specified file.
 * 
 * @param bpb BPB structure
 * 
 * @return Upon success zero is returned. Otherwise negative value is
 * returned and error is indicated in errno.
 */
int
fat32_bpb_verbose_info(const struct fat32_bpb_t *bpb);


/** 
 * Checks whether a bpb structure is a correct FAT32 BPB
 * 
 * @param bpb a structure to check
 * 
 * @return @em true if correct. @em false otherwise.
 */
bool
fat32_bpb_check_validity(const struct fat32_bpb_t *bpb);

/** 
 * Reads BPB structure from file and validates it.
 * 
 * @param fd File descriptor used to read BPB. As this function
 *           is assumed to be used only once during driver initialization
 *           so we do not do any @em lseek and require for file offset to
 *           be exactly befory BPB on the disk.
 * @param bpb A pointer to structure where read information must be stored.
 * 
 * @retval FE_OK
 * @retval FE_ERRNO unable to read information from underlying device file
 * @retval FE_INVALID_DEV underlying device file ended prematurely
 * @retval FE_INVALID_FS data in the BPB block of device is inconsistent
 */
enum fat32_error_t
fat32_bpb_read(int fd, struct fat32_bpb_t *bpb);

/** 
 * Calculates the number of clusters on the file system.
 * 
 * @param bpb BPB of the file system
 * 
 * @return number of clusters
 */
uint32_t
fat32_bpb_clusters_count(const struct fat32_bpb_t *bpb);

#endif /* _BPB_H_ */
