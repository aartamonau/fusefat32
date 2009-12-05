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

#include <stdbool.h>

#include <inttypes.h>
#include <unistd.h>

#include "fat32/errors.h"
#include "utils/inlines.h"

/// a size of the @em name field in #fat32_direntry_t
#define FAT32_DIRENTRY_NAME_SIZE 11

/// a size of extension in #fat32_direntry_t::name
#define FAT32_DIRENTRY_EXTENSION_SIZE 3

/// a size of the name without extension in the #fat32_direntry_t::name
#define FAT32_DIRENTRY_BASE_NAME_SIZE 8

/// type representing directory entry attributes
typedef uint8_t fat32_direntry_attr_t;

/// file is read only
#define FAT32_DIRENTRY_READ_ONLY ((fat32_direntry_attr_t) 0x01)

/// file is hidden
#define FAT32_DIRENTRY_HIDDEN ((fat32_direntry_attr_t) 0x02)

/// system file
#define FAT32_DIRENTRY_SYSTEM ((fat32_direntry_attr_t) 0x04)

/// file specifies volume id
#define FAT32_DIRENTRY_VOLUME_ID ((fat32_direntry_attr_t) 0x08)

/// directory
#define FAT32_DIRENTRY_DIRECTORY ((fat32_direntry_attr_t) 0x10)

/// archived flag
#define FAT32_DIRENTRY_ARCHIVE ((fat32_direntry_attr_t) 0x20)

/// part of long file name
#define FAT32_DIRENTRY_LONG_NAME \
  (FAT32_DIRENTRY_READ_ONLY | FAT32_DIRENTRY_HIDDEN | \
   FAT32_DIRENTRY_SYSTEM | FAT32_DIRENTRY_VOLUME_ID)

/// type representing time as it's stored on FAT file system
typedef uint16_t fat32_time_t;

/// type representing date as it's stored on FAT file system
typedef uint16_t fat32_date_t;

/// Directory entry structure
struct fat32_direntry_t {
  uint8_t               name[FAT32_DIRENTRY_NAME_SIZE]; /**< short name */
  fat32_direntry_attr_t attr;     /**< directory entry attributes */
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

/**
 * Determines whether direntry is empty.
 *
 * @param direntry direntry to check
 *
 * @return boolean value showing whether direnty is empty
 */
bool
fat32_direntry_is_empty(const struct fat32_direntry_t *direntry);

/**
 * Determines whether direntry is last in the directory.
 *
 * @param direntry direntry to check
 *
 * @return boolean value showing whether direnty is actually the last
 */
bool
fat32_direntry_is_last(const struct fat32_direntry_t *direntry);

/**
 * Determines whether direntry has specified attribute.
 *
 * @param direntry direntry
 * @param attr attribute to check
 *
 * @return boolean value showing whether direntry has given attribute
 */
INLINE bool
fat32_direntry_has_attr(const struct fat32_direntry_t *direntry,
                        fat32_direntry_attr_t attr)
{
  return direntry->attr & attr;
}

/**
 * Determines whether direntry corresponds to ordinary file.
 *
 * @param direntry direntry to check
 *
 * @return boolean result of the check
 */
INLINE bool
fat32_direntry_is_file(const struct fat32_direntry_t *direntry)
{
  return !(fat32_direntry_has_attr(direntry, FAT32_DIRENTRY_DIRECTORY) ||
           fat32_direntry_has_attr(direntry, FAT32_DIRENTRY_VOLUME_ID) ||
           fat32_direntry_has_attr(direntry, FAT32_DIRENTRY_LONG_NAME));
}

/**
 * Determines whether direntry corresponds to some directory.
 *
 * @param direntry directory entry to check
 *
 * @return result
 */
INLINE bool
fat32_direntry_is_directory(const struct fat32_direntry_t *direntry)
{
  return fat32_direntry_has_attr(direntry, FAT32_DIRENTRY_DIRECTORY);
}

/**
 * Returns a short name of the object specified by directory entry.
 *
 * @param direntry directory entry
 *
 * @return A string containing short name. Must be freed manually.
 *         NULL is returned if it's insufficient memory to allocate
 *         a buffer for name.
 */
char *
fat32_direntry_short_name(const struct fat32_direntry_t *direntry);

#endif /* _DIRENTRY_H_ */
