/**
 * @file   fat32/errors.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 17:44:58 2009
 * 
 * @brief  Defines errors that can appear while working with filesystem.
 *         Separated to make it usable by the whole program.
 * 
 * 
 */
#ifndef _FS_ERRORS_H_
#define _FS_ERRORS_H_

enum fat32_error_t {
  FE_OK,                        /**< no errors occured */
  FE_ERRNO,                     /**< indicates that errno specifies error */
  FE_NONBLOCK_DEV,              /**< invalid device is not a block device */
  FE_INVALID_DEV,               /**< invalid device (too small and other
				   errors of that kind)
                                */
  FE_INVALID_FS			/**< invalid filesystem */
};

#endif /* _FS_ERRORS_H_ */
