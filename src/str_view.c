#include "str_view.h"

#include <printf.h>
/* See: https://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html
 * for more information on how to implement custom printf format specifiers.
 */

__HIDDEN int
_str_view_printf(FILE *stream, const struct printf_info *info, const void *const *args) {
    str_view_t sv = *((str_view_t *) args[0]);
    return fprintf(stream, "%.*s", sv.len, sv.str);
}

__HIDDEN int 
_str_view_arginfo(const struct printf_info *info, size_t n, int *argtypes) {
    if (n > 0) {
        argtypes[0] = PA_POINTER;
    }
    return 1;
}

#if defined(__GNUC__) || defined(__clang__)
#define HAS_CONSTRUCTOR_ATTRIBUTE
#endif

#ifdef HAS_CONSTRUCTOR_ATTRIBUTE
__attribute__ ((constructor)) void
str_view_printf_init(void) {
    register_printf_function('v', _str_view_printf, _str_view_arginfo);
}
#else
#error "Constructor attribute is not supported"
#endif


