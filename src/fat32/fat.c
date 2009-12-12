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

/**
 * Finds an offset of the FAT entry corresponding to the specified cluster
 * on the device.
 *
 * @param fat     FAT object.
 * @param cluster Cluster number.
 *
 * @return Offset.
 */
static off_t
fat32_fat_entry_offset(const struct fat32_fat_t *fat, uint32_t cluster);

enum fat32_error_t
fat32_fat_init(struct fat32_fat_t *fat,
               const struct fat32_fs_t *fs)
{
  int fd = dup(fs->fd);
  if (fd < 0) {
    return FE_ERRNO;
  }

  fat->fd                = fd;
  fat->bpb               = fs->bpb;
  fat->fs_info           = fs->fs_info;
  /* we set a hint to the minimum possible cluster number as opposite to
   * free cluster hint from fsinfo because the latter can contain incorrect
   * information */
  fat->free_cluster_hint = FAT32_MIN_CLUSTER_NUMBER;

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

off_t
fat32_fat_entry_offset(const struct fat32_fat_t *fat, uint32_t cluster)
{
  /* an offset of the entry in a FAT corresponding to @em cluster */
  uint32_t entry_fat_offset    = cluster * FAT32_FAT_ENTRY_SIZE;
  uint32_t entry_sector        =
    fat->bpb->reserved_sectors_count +
    (entry_fat_offset / fat->bpb->bytes_per_sector);
  uint32_t entry_sector_offset = entry_fat_offset % fat->bpb->bytes_per_sector;

  return fat32_sector_offset_to_offset(fat->bpb,
                                       entry_sector,
                                       entry_sector_offset);
}

enum fat32_error_t
fat32_fat_get_entry(const struct fat32_fat_t *fat,
                    uint32_t cluster, fat32_fat_entry_t *entry)
{
  off_t offset = fat32_fat_entry_offset(fat, cluster);

  off_t ret = lseek(fat->fd, offset, SEEK_SET);
  if (ret == (off_t) -1) {
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

enum fat32_error_t
fat32_fat_get_nth_entry(const struct fat32_fat_t *fat,
                        uint32_t cluster, uint32_t n,
                        fat32_fat_entry_t *entry)
{
  *entry     = cluster;
  uint32_t i = 0;
  while (i < n) {
    enum fat32_error_t ret = fat32_fat_get_entry(fat, cluster, entry);
    if (ret != FE_OK) {
      return ret;
    }

    if (fat32_fat_entry_is_null(*entry)) {
      return FE_CLUSTER_CHAIN_ENDED;
    }

    cluster = fat32_fat_entry_to_cluster(*entry);
    if (fat32_fat_entry_is_bad(*entry) ||
        fat32_fat_entry_is_free(*entry)) {
      return FE_INVALID_FS;
    }
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
fat32_fat_entry_is_free(fat32_fat_entry_t entry)
{
  return (entry & FAT32_FAT_ENTRY_MASK) == 0;
}

enum fat32_error_t
fat32_fat_find_free_cluster(struct fat32_fat_t *fat, uint32_t *cluster)
{
  uint32_t candidate = fat->free_cluster_hint;
  uint32_t total     =
    fat32_bpb_clusters_count(fat->bpb) + FAT32_MIN_CLUSTER_NUMBER;

  while (candidate < total) {
    fat32_fat_entry_t  entry;
    enum fat32_error_t ret = fat32_fat_get_entry(fat, candidate, &entry);
    if (ret != FE_OK) {
      return ret;
    }

    if (fat32_fat_entry_is_free(entry)) {
      fat->free_cluster_hint = candidate;
      return candidate;
    }

    ++candidate;
  }
  return FE_FS_IS_FULL;
}
