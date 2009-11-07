/**
 * @file   utils.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 18:34:40 2009
 * 
 * @brief  Various fs-related utility functions.
 * 
 * 
 */
#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <inttypes.h>

#include "fat32/bpb.h"

#ifndef INLINE
  #define INLINE inline
#endif

/** 
 * Transforms a number of sector to file offset.
 * 
 * @param bpb BPB structure
 * @param sector sector number
 * 
 * @return offset in file
 */
INLINE off_t
sector_to_offset(const struct fat32_bpb_t *bpb, uint32_t sector)
{
  return (off_t) bpb->bytes_per_sector * (off_t) sector;
}



#endif /* _UTILS_H_ */
