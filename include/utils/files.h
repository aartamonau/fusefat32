/**
 * @file   files.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 23:55:01 2009
 * 
 * @brief  Utility functions working with files.
 * 
 * 
 */

#ifndef _FILES_H_
#define _FILES_H_

#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

/** 
 * Closes file descriptor retrying if EINTR occures. All other errors
 * are indicated as usual.
 * 
 * @param fd file descriptor to close
 * 
 * @return 0 on success, -1 otherwise
 */
int
eintr_safe_close(int fd);

/** 
 * Analogue of standard @em read system call which ensures
 * that all requested data is read by one call (if it's possible).
 *
 * NB: If error is returned by this function then the position of
 * cursor in file is unspecified. Actually some of the data could
 * have been read successfully.
 * 
 * @param fd file descriptor
 * @param buf a buffer to store readed information
 * @param count number of bytes to read
 * 
 * @return Number of bytes read. Zero indicates the end of file.
 *         Error is indicated by -1 value.
 */
ssize_t
xread(int fd, void *buf, size_t count);

/** 
 * Analogue of standard @em write system call which ensures
 * that all requested data is written by one call (if it's possible).
 * 
 * NB: If error is returned by this function then the position of
 * cursor in file is unspecified. Actually some of the data could
 * have been written to the file by the time of error occurence.
 * 
 * @param fd file descriptor
 * @param buf a buffer with information
 * @param count number of bytes to read
 * 
 * @return Number of bytes written. Zero
 */
ssize_t
xwrite(int fd, void *buf, size_t count);

#endif
