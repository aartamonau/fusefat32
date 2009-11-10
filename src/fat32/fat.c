/**
 * @file   fat.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Tue Nov 10 01:08:13 2009
 * 
 * @brief  Implementation of functions working with file allocation tables.
 * 
 * 
 */
#include <unistd.h>

#include "utils/files.h"

#include "fat32/fs.h"
#include "fat32/fat.h"
#include "fat32/utils.h"

enum fat32_error_t
fat32_fat_init(struct fat32_fat_t *fat,
	       const struct fat32_fs_t *fs)
{
  int fd = dup(fs->fd);
  if (fd < 0) {
    return FE_ERRNO;
  }

  fat->fd      = fd;
  fat->bpb     = fs->bpb;
  fat->fs_info = fs->fs_info;

  fat->bytes_per_sector_log =
    fat32_highest_bit_number(fs->bpb->bytes_per_sector);

  return FE_OK;
}

enum fat32_error_t
fat32_fat_finalize(struct fat32_fat_t *fat)
{
  if (xclose(fat->fd) < 0) {
    return FE_ERRNO;
  }

  return FE_OK;
}

/// a size in bytes of entry in file allocation table
static const uint8_t  FAT32_FAT_ENTRY_SIZE = sizeof(fat32_fat_entry_t);

enum fat32_error_t
fat32_fat_get_entry(const struct fat32_fat_t *fat,
		    uint32_t cluster, fat32_fat_entry_t *entry)
{
  /* an offset of the entry in a FAT corresponding to @em cluster */
  uint32_t entry_fat_offset    = cluster * FAT32_FAT_ENTRY_SIZE;
  uint32_t entry_sector        = fat->bpb->reserved_sectors_count + \
    (entry_fat_offset / fat->bpb->bytes_per_sector);
  uint32_t entry_sector_offset = entry_fat_offset % fat->bpb->bytes_per_sector;

  off_t file_offset = fat32_sector_offset_to_offset(fat->bpb,
						    entry_sector,
						    entry_sector_offset);

  off_t ret = lseek(fat->fd, file_offset, SEEK_SET);
  if (ret < (off_t) 0) {
    return FE_ERRNO;
  }
		    
  ssize_t nread = xread(fat->fd, entry, FAT32_FAT_ENTRY_SIZE);
  if (nread >= 0) {
    if (nread < FAT32_FAT_ENTRY_SIZE) {
      return FE_INVALID_DEV;
    }
  } else {
    return FE_ERRNO;
  }

  return FE_OK;
}

/// end of cluster chain mark
static const fat32_fat_entry_t FAT32_FAT_ENTRY_EOC = 0x0ffffff8;
bool
fat32_fat_entry_is_null(fat32_fat_entry_t entry)
{
  return entry >= FAT32_FAT_ENTRY_EOC;
}

/// mask that matches 28-bits in 32-bit fat entry which are actually are
/// used
static const fat32_fat_entry_t FAT32_FAT_ENTRY_MASK = 0x0fffffff;

/// bad cluster fat entry mark
static const uint32_t FAT32_FAT_ENTRY_BAD           = 0x0ffffff7;
bool
fat32_fat_entry_is_bad(fat32_fat_entry_t entry)
{
  return (entry & FAT32_FAT_ENTRY_MASK) == FAT32_FAT_ENTRY_BAD;
}

uint32_t
fat32_fat_entry_to_cluster(fat32_fat_entry_t entry)
{
  return (uint32_t) (entry & FAT32_FAT_ENTRY_MASK);
}

bool
fat32_fat_cluster_is_free(uint32_t cluster)
{
  return cluster == 0;
}


