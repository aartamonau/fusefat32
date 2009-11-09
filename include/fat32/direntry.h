/**
 * @file   direntry.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Nov  9 15:11:23 2009
 * 
 * @brief  Structures and functions related to FAT's directory entries.
 * 
 * 
 */
#ifndef _DIRENTRY_H_
#define _DIRENTRY_H_

#include <inttypes.h>
#include <unistd.h>

#include "fat32/errors.h"

#define FAT32_DIRENTRY_NAME_SIZE 11

/// type representing directory entry attributes
typedef uint8_t fat32_direntry_attr_t;

/// type representing time as it's stored on FAT file system
typedef uint16_t fat32_time_t;

/// type representing date as it's stored on FAT file system
typedef uint16_t fat32_date_t;

/// Directory entry structure
struct fat32_direntry_t {
  uint8_t               name[FAT32_DIRENTRY_NAME_SIZE]; /**< short name */
  fat32_direntry_attr_t attr;	  /**< directory entry attributes */
  uint8_t               reserved; /**< reserved for NT */
  uint8_t               creation_time_tenth; /**< tenth of the second of
						creation time */
  fat32_time_t          creation_time; /**< creation time */
  fat32_date_t          creation_date; /**< creation date */
  fat32_date_t          access_date;   /**< last access date */

  uint16_t              first_cluster_hi; /**< high word of entry's first
					     cluster number */

  fat32_time_t          write_time; /**< the time of last write */
  fat32_date_t          write_date; /**< the date of last write */

  uint16_t              first_cluster_lo; /**< low word of entry's first
					     cluster number */
  uint32_t              file_size; /**< file size (zero for directory) */
} __attribute__((packed));

#endif /* _DIRENTRY_H_ */
