#include "shared.h"

#ifdef _DEBUG

#define debug_print(fmt, ...) \
            do { fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                    __LINE__, __func__, __VA_ARGS__); } while (0)

#define debug_assert(x) \
            do { if (!(x)) { fprintf(stderr, "%s:%d:%s(): " "assertion failed: %s\n", __FILE__, \
                                    __LINE__, __func__, #x); } } while (0)

#else

#define debug_print(fmt, ...) \
            do { } while (0)

#define debug_assert(x) \
            do { } while (0)

#endif

// universal debug print

#define raise_error(fmt, ...) \
            do { fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                    __LINE__, __func__, __VA_ARGS__); } while (0)

