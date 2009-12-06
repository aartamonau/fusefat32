/**
 * @file   operations.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sun Dec  6 04:35:33 2009
 *
 * @brief  FUSE operations
 *
 *
 */

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include <fuse.h>

/// fuse operations
extern const struct fuse_operations fusefat32_operations;

#endif /* _OPERATIONS_H_ */
