/**
 * @file   file_info.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Dec 12 12:09:56 2009
 *
 * @brief  Information about open files.
 *
 * @todo: move to fusefat32 directory
 *
 *
 */
#ifndef _FILE_INFO_H_
#define _FILE_INFO_H_

#include <stdbool.h>

/// Structure storing information about open files.
struct fat32_file_info_t {
  unsigned int refs;            /**< number of open instances */
  bool         deleted;         /**< set to @em true if file is deleted
                                 *   while it's open. So that the last
                                 *   @em release call can perform actual
                                 *   deletion. */
};

/**
 * A cloner for #fat32_file_info_t structures. For now it does not use the
 * value supplied through the argument. Just creates a new structure
 * initialized appropriately.
 *
 * @param file_info Unused.
 *
 * @return New structure. NULL on error.
 */
void *
fat32_file_info_cloner(const void *file_info);

#endif /* _FILE_INFO_H_ */
