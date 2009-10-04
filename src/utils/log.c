/**
 * @file   log.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Oct  3 22:40:38 2009
 * 
 * @brief  Logging utitlity functions 
 * 
 * @link log_message @endlink that actually logs messages of course
 * locks on @link log_lock @endlink before writing something to
 * @link log_file @endlink. But there is another place for synchronization
 * in this function. It is a check of whether @link log_file @endlink has
 * non NULL value. And this check is not interlocked to not introduce locking
 * overhead when logging, for instance, disabled at all. This behavior is
 * correct only because we insist that @link log_init_from_path @enlink,
 * @link log_init_from_file @endlink and @link log_close @endlink must be
 * called when any call to @link log_message @endlink (and macros which use it)
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
 * be closed by @link log_close @endlink call.
 * 
 */
static bool restricted;

/// minimum importance level a message must have to be logged
static enum log_level_t log_level;

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
    int ioret = fprintf(log_file, "%s: ", LEVEL_MESSAGES[level]);
    if (ioret < 0) {
      result = -1;
      goto log_message_unlock;
    }

    va_list arglist;
    va_start(arglist, format);
    ioret = vfprintf(log_file, format, arglist);
    va_end(arglist);

    if (ioret < 0) {
      result = -1;
      goto log_message_unlock;
    }

    ioret = fputs("\n", log_file);
    if (ioret < 0) {
      result = -1;
      goto log_message_unlock;
    }
  }

 log_message_unlock:
  assert( pthread_mutex_unlock(&log_lock) == 0 );
  return result;
}
