/**
 * @file   log.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Oct  3 16:40:14 2009
 *
 * @brief  Logging utilities.
 *
 * There is a tricky point about error handling in the functions
 * exposed in this header. As @em stdio.h is used internally so
 * stictly c99 compliant standard library may not set @em errno
 * accordingly to errors occured. But as @em _GNU_SOURCE is needed
 * to compile against FUSE we ignore this problem (at least for now).
 */

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

/**
 * Importance of log messages. Types are almost the same as @em syslog
 * uses. The only difference is that two most important message types
 * are not included here.
 *
 */
enum log_level_t {
  LOG_DEBUG,      /**< debug-level message */
  LOG_INFO,       /**< informational message */
  LOG_NOTICE,     /**< normal, but significant, condition */
  LOG_WARNING,    /**< warning conditions */
  LOG_ERROR,      /**< error conditions */
  LOG_CRITICAL    /**< critical conditions */
};

/**
 * Initialize logging using a @em FILE object to which all messages will be
 * sent.
 *
 * CAUTION: This function is not thread safe as it's intended to be used
 * only once during program initialization.
 *
 * @param file a @em FILE to send messages to
 * @param level a minimum level a message must have to be logged
 *
 */
void
log_init_from_file(FILE *file, enum log_level_t level);

/**
 * Initialize logging using a path to file where messages will be stored.
 *
 * CAUTION: This function is not thread safe as it's intended to be used
 * only once during program initialization.
 *
 * @param path a path to file where messages will be stored
 * @param level a minimum level a message must have to be logged
 *
 * @return 0 is returned on success. -1 means that error occured. Error type is
 * indicated by @em errno.
 */
int
log_init_from_path(const char *path, enum log_level_t level);

/**
 * Close logging gracefully.
 *
 *
 * @return 0 on success. -1 in case of error. Error type is indicated by
 * @em errno
 */
int
log_close(void);

/**
 * Log a message. Must be called after logging has been initialized.
 *
 * @param level importance level of the message
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
int
log_message(enum log_level_t level, const char *format, ...);

/**
 * Log a message of LOG_DEBUG importance level.
 *
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
#define log_debug(format, ...)      \
  log_message(LOG_DEBUG, format, ##__VA_ARGS__)

/**
 * Log a message of LOG_INFO importance level.
 *
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
#define log_info(format, ...)       \
  log_message(LOG_INFO, format, ##__VA_ARGS__)

/**
 * Log a message of LOG_NOTICE importance level.
 *
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
#define log_notice(format, ...)       \
  log_message(LOG_NOTICE, format, ##__VA_ARGS__)

/**
 * Log a message of LOG_WARNING importance level.
 *
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
#define log_warning(format, ...)    \
  log_message(LOG_WARNING, format, ##__VA_ARGS__)

/**
 * Log a message of LOG_ERROR importance level.
 *
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
#define log_error(format, ...)      \
  log_message(LOG_ERROR, format, ##__VA_ARGS__)

/**
 * Log a message of LOG_CRITICAL importance level.
 *
 * @param format @em printf format string
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
#define log_critical(format, ...)     \
  log_message(LOG_CRITICAL, format, ##__VA_ARGS__)

#endif /* _LOG_H_ */
