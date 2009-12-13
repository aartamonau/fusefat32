/**
 * @file   log.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Oct  3 22:40:38 2009
 *
 * @brief  Logging utitlity functions
 *
 * ::log_message that actually logs messages of course locks on
 * #log_lock before writing something to #log_file. But there is
 * another place for synchronization in this function. It is a check of
 * whether #log_file has a non NULL value. And this check is not interlocked
 * to not introduce locking overhead when logging, for instance, disabled at
 * all. This behavior is correct only because we insist that
 * ::log_init_from_path, ::log_init_from_file and ::log_close must be
 * called when any call to ::log_message (and macros which use it)
 * is strictly impossible.
 * The same applies to the check of message's log level.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <pthread.h>

#include "utils/log.h"

/// a mutex to lock on when writing messages
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

/// FILE object to which messages are sent
static FILE *log_file = NULL;

/**
 * If this variable is @em true then it means that file can
 * be closed by ::log_close call.
 *
 */
static bool restricted;

/// minimum importance level a message must have to be logged
static enum log_level_t log_level;

/// defines the messages that are displayed before logged text for
/// each log level
static const char *LEVEL_MESSAGES [] = { [LOG_DEBUG]    = "DEBUG",
                                         [LOG_INFO]     = "INFO",
                                         [LOG_NOTICE]   = "NOTICE",
                                         [LOG_WARNING]  = "WARNING",
                                         [LOG_ERROR]    = "LOG_WARNING",
                                         [LOG_CRITICAL] = "CRITICAL" };



void
log_init_from_file(FILE *file, enum log_level_t level)
{
  log_file   = file;
  restricted = false;
  log_level  = level;
}

int
log_init_from_path(const char *path, enum log_level_t level)
{
  FILE *file = fopen(path, "w");
  if (file == NULL) {
    return -1;
  }

  log_file   = file;
  restricted = true;
  log_level  = level;

  return 0;
}

int
log_close(void)
{
  if (log_file != NULL) {
    if (restricted) {
      if (fclose(log_file) != 0) {
  return -1;
      }
    }
  }

  return 0;
}

/**
 * Does actual job needed to log some message. Does not acquire log lock.
 *
 * @param level   Importance level.
 * @param format  Format string.
 * @param arglist Argument list.
 *
 * @return 0 on success. -1 on error. Error type is indicated by @em errno.
 */
static int
do_log_message(enum log_level_t level, const char *format, va_list arglist)
{
  int ioret = fprintf(log_file, "%s: ", LEVEL_MESSAGES[level]);
  if (ioret < 0) {
    return -1;
  }

  ioret = vfprintf(log_file, format, arglist);
  if (ioret < 0) {
    return -1;
  }

  ioret = fflush(log_file);
  if (ioret == EOF) {
    return -1;
  }

  ioret = fputs("\n", log_file);
  if (ioret < 0) {
    return -1;
  }

  return 0;
}

int
log_message(enum log_level_t level, const char *format, ...)
{
  if (log_file == NULL) {
    return 0;
  }

  if (level < log_level) {
    return 0;
  }

  int result = 0;

  int ret    = pthread_mutex_lock(&log_lock);
  if (ret == 0) {
    va_list arglist;
    va_start(arglist, format);

    int ioret = do_log_message(level, format, arglist);
    va_end(arglist);

    result = ioret;
  } else {
    return -1;
  }

  assert( pthread_mutex_unlock(&log_lock) == 0 );
  return result;
}

int
log_message_loc(enum log_level_t level,
                const char *file, const char *function,
                const char *format, ...)
{
  if (log_file == NULL) {
    return 0;
  }

  if (level < log_level) {
    return 0;
  }

  int result = 0;

  int ret = pthread_mutex_lock(&log_lock);
  if (ret == 0) {
    int ioret = fprintf(log_file, "%s : %s - ", file, function);
    if (ioret < 0) {
      result = -1;
      goto cleanup;
    }

    va_list arglist;
    va_start(arglist, format);

    ioret = do_log_message(level, format, arglist);
    va_end(arglist);

    result = ioret;
  } else {
    return -1;
  }
cleanup:
  assert( pthread_mutex_unlock(&log_lock) == 0 );
  return result;
}
