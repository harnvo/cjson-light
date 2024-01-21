#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#if !defined(_C_JSON_H_)
#define _C_JSON_H_
#endif

#if defined(__cplusplus)
#define __HEADER_ONLY inline
#else
#define __HEADER_ONLY static inline
#endif

// macro for visibility
#if ( (defined(__GNUC__) &&__GNUC__>=4 ) || defined(__SUNPRO_CC) || defined(__SUNPRO_C))
// #define __HIDDEN __attribute__ ((visibility ("hidden")))
// #define __EXPOSED __attribute__ ((visibility ("default")))
// #else
#define __HIDDEN
#define __EXPOSED
#endif

// cross patform


// nan and inf
/* https://github.com/DaveGamble/cJSON/blob/master/cJSON.c#L72 */
/* define isnan and isinf for ANSI C, if in C99 or above, isnan and isinf has been defined in math.h */
#ifndef isinf
#define isinf(d) (isnan((d - d)) && !isnan(d))
#endif
#ifndef isnan
#define isnan(d) (d != d)
#endif

#ifndef _json_num_nan
#ifdef _WIN32
#define _json_num_nan sqrt(-1.0)
#else
#define _json_num_nan 0.0/0.0
#endif
#endif

// locks
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <pthread.h>
#define USE_PTHREAD_LOCK

#define _lock_t pthread_mutex_t
#define _lock_init(lock) pthread_mutex_init (lock, NULL)
#define _lock_destroy(lock) pthread_mutex_destroy (lock)
#define _lock(lock) pthread_mutex_lock (lock)
#define _unlock(lock) pthread_mutex_unlock (lock)

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define USE_WIN_LOCK

#define _lock_t CRITICAL_SECTION
#define _lock_init(lock) InitializeCriticalSection (lock)
#define _lock_destroy(lock) DeleteCriticalSection (lock)
#define _lock(lock) EnterCriticalSection (lock)
#define _unlock(lock) LeaveCriticalSection (lock)

#error "Windows is not tested"

#else
#error "Unsupported platform"
#endif

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

// some global hooks
// need to used with lock

// there is one more concern: might not work when json_err is written by
// another thread immediately after the lock released.
struct json_err {
  int line;
  int column;
  char *source;
  char *text;

  // for internal use
  _lock_t lock;
};

static thread_local struct json_err global_json_error;

static void
json_err_print (struct json_err *err) {
  fprintf (stderr,
           "Error at line %d, column %d\n",
           err->line, err->column);
  fprintf (stderr, "Source: %s\n", err->source);
  if (err->text) {
    fprintf (stderr, "Text: %s\n", err->text);
  }
}

static void
json_err_type_not_supported (int line, int column, int position,
                             int position_at_line, const char *type) {
  static char *err_msg = "Type %s not supported";

  struct json_err *err = &global_json_error;
  _lock (&err->lock);
  err->line = line;
  err->column = column;
  char buf[sizeof(err_msg)+sizeof(type)];
  sprintf (buf, err_msg, type);
  err->source = buf;
  json_err_print (err);
  _unlock (&err->lock);
}

// malloc&free
typedef void *(*json_malloc_t) (size_t);
typedef void (*json_free_t) (void *);

static int json_default_malloc = 1;
// json_malloc_t json_malloc = malloc;
// json_free_t json_free = free;

struct global_hooks {
  json_malloc_t malloc_fn;
  json_free_t free_fn;
}

static json_global_hooks = {malloc, free};

#define INIT_JSON_MALLOC(malloc_fx, free_fx)                                  \
  do {                                                                         \
    json_global_hooks.malloc_fn = malloc_fx;                                  \
    json_global_hooks.free_fn = free_fx;                                      \
  } while (0)



// allocator for cpp
#if defined(__cplusplus)
#include <memory>
#include <new>

template <typename _Tp> class json_allocator : public std::allocator<_Tp> {};

#endif


