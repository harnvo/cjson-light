#pragma once
#include "shared.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

// Path: json-c/json.h

#if defined(__cplusplus)
extern "C" {
#endif

/* Before we start, provide some basic cstr utils */
__HEADER_ONLY
char *
strskip (const char *str, size_t __max_len) {
  while (*str && __max_len) {
    if (isspace (*str) || *str == '\n' || *str == '\r' || *str == '\t')
      {
        str++;
        __max_len--;
      }
    else
      break;
  }

  return (char *)str;
}

__HEADER_ONLY
int
stricmp (const char *s1, const char *s2) {
  while (*s1 && *s2) {
    int diff = tolower (*s1) - tolower (*s2);
    if (diff)
      return diff;
    s1++;
    s2++;
  }

  return *s1 - *s2;
}

__HEADER_ONLY
int
strnicmp (const char *s1, const char *s2, size_t n) {
  while (*s1 && *s2 && n) {
    int diff = tolower (*s1) - tolower (*s2);
    if (diff)
      return diff;
    s1++;
    s2++;
    n--;
  }

  return *s1 - *s2;
}

__HEADER_ONLY
int
stoi (const char *str, int *out) {
  return sscanf (str, "%d", out);
}

__HEADER_ONLY
int
stof (const char *str, float *out) {
  return sscanf (str, "%f", out);
}

__HEADER_ONLY
int
stod (const char *str, double *out) {
  return sscanf (str, "%lf", out);
}

/* ----------------------------------------------- */

// string view

struct str_view {
  char *str;
  size_t len;
};

struct kv_pair {
  struct str_view key;
  struct str_view value;
};

typedef struct str_view str_view_t;
typedef struct kv_pair kv_pair_t;

// init
__HEADER_ONLY
int str_view_init (struct str_view *sv, const char *str, size_t len);

__HEADER_ONLY
int str_view_init_from_str (struct str_view *sv, const char *str);

__HEADER_ONLY
int str_view_init_from_substr (struct str_view *sv, const char *str,
                               size_t start, size_t end);

// print
__HEADER_ONLY
int str_view_print (const struct str_view *sv);

// parser. VERY USEFUL
__HEADER_ONLY
int str_view_parse_str (struct str_view *dst, struct str_view src);

// trim. Ignore the space at the beginning and end of the string
__HEADER_ONLY
int str_view_trim (struct str_view *sv);

__HEADER_ONLY
int str_view_ltrim (struct str_view *sv);

__HEADER_ONLY
int str_view_rtrim (struct str_view *sv);

// substr
__HEADER_ONLY
int str_view_substr_ (struct str_view *sv, size_t start, size_t end);

__HEADER_ONLY
int str_view_substr (struct str_view *src, struct str_view *dst, size_t start,
                     size_t end);

// compare. i for case insensitive
__HEADER_ONLY
int str_view_cmp (const struct str_view sv1, const struct str_view sv2);

__HEADER_ONLY
int str_view_ncmp (const struct str_view sv1, const struct str_view sv2,
                   size_t n);

__HEADER_ONLY
int str_view_icmp (const struct str_view sv1, const struct str_view sv2);

__HEADER_ONLY
int str_view_nicmp (const struct str_view sv1, const struct str_view sv2,
                    size_t n);

// find
__HEADER_ONLY
int str_view_findc (const struct str_view *sv, const char str);

__HEADER_ONLY
int str_view_findsv (const struct str_view *sv, const struct str_view *sv2);

// converters
__HEADER_ONLY
int str_view_toi (const struct str_view *sv, int *out);

// __HEADER_ONLY
// int str_view_tof (const struct str_view *sv, float *out);


/* ----------------------------------------------- */

/* ----------------------- */
/* --- implementations --- */
/* ----------------------- */

__HEADER_ONLY int
str_view_init (struct str_view *sv, const char *str, size_t len) {
  sv->str = (char *)str;
  sv->len = len;
  return 0;
}

__HEADER_ONLY int
str_view_init_from_str (struct str_view *sv, const char *str) {
  return str_view_init (sv, str, strlen (str));
}

__HEADER_ONLY int
str_view_init_from_substr (struct str_view *sv, const char *str, size_t start,
                           size_t end) {
  return str_view_init (sv, str + start, end - start);
}

// print
__HEADER_ONLY int
str_view_print (const struct str_view *sv) {
  return printf ("%.*s", (int)sv->len, sv->str);
}

// parser. VERY USEFUL
// see `json_obj_parse_str`
__HIDDEN __HEADER_ONLY int
__sv_parse_str_loop (char **ch_ptr, int *ctx) {
	const static int CTX_backslash = 1 << 0;
  const static int CTX_unicode = 1 << 1;

  char *ch = *ch_ptr;
	// check for context
	if (*ctx & CTX_backslash) {
		switch (*ch) {
      case '\"': case '\\': case '/': case 'b':
      case 'f':  case 'n':  case 'r': case 't': {
        *ctx &= ~CTX_backslash;
        break;
      } case 'u': {
        *ctx &= ~CTX_backslash;
        *ctx |= CTX_unicode;
        break;
      } default: {
        return -1;
      }
		}
    // ch++;
    *ch_ptr += 1;
		return 0;
	}

	if (*ctx & CTX_unicode) {
		for (int i = 0; i < 4; i++) {
			if (!isxdigit(*ch)) {
        printf("error: %c\n", *ch);
				return -1;
			}
			ch++;
      *ch_ptr += 1;
		}
    *ctx &= ~CTX_unicode;
		return 0;
	}

	// normal context
	// deal with backslash
	if (*ch == '\\') {
		// ch++;
    *ch_ptr += 1;
		*ctx |= CTX_backslash;
		return 0;
	}

	if (*ch == '\"') {
		return 1;
	}
  // ch++;
  *ch_ptr += 1;

}

// do the parsing assuming the first char is '\"'
__EXPOSED __HEADER_ONLY int
__str_view_parse_str (str_view_t *dst, str_view_t src) {
	if (src.len == 0) {
		return -1;
	}

	char *ch = src.str;
	char *ch_end = ch + src.len - 1;

	// assume the first char is '\"'
	ch++;
  dst->str = ch;

	int __ctx = 0;
	while ((*ch != '\0' && *ch != '\r' && *ch != '\n'
         && *ch != *ch_end) || __ctx != 0) {
		/* code */
		int flag = __sv_parse_str_loop (&ch, &__ctx);
    
		if (flag == -1) {
			return -1;
		}

		if (flag == 1) {
			break;
		}
	}

	if (*ch != '\"') {
		return -1;
	}
	
	dst->len = ch - dst->str;
  if (dst->len == 0) {
    return -1;
  }
  // printf("%l\n", (long) dst->len);
  return 0;
}

// check before parsing. See `__str_view_parse_str`
__EXPOSED __HEADER_ONLY int
str_view_parse_str (str_view_t *dst, str_view_t src) {
  if (src.len == 0) {
    return -1;
  }

  // ignore leading spaces
  char *ch = src.str;
  while (isspace (*ch) && src.len) {
    ch++;
    src.len--;
  }

  if (*ch != '\"') {
    return -1;
  }

  src.str = ch;

  return __str_view_parse_str (dst, src);
}

// trim
__HEADER_ONLY int
str_view_trim (struct str_view *sv) {
  str_view_ltrim (sv);
  str_view_rtrim (sv);
  return 0;
}

__HEADER_ONLY int
str_view_ltrim (struct str_view *sv) {
  while (isspace (*sv->str) && sv->len) {
    sv->str++;
    sv->len--;
  }

  return 0;
}

__HEADER_ONLY int
str_view_rtrim (struct str_view *sv) {
  while (isspace (sv->str[sv->len - 1]) && sv->len) {
    sv->len--;
  }

  return 0;
}

// substr
__HEADER_ONLY int
str_view_substr_ (struct str_view *sv, size_t start, size_t end) {
  if (start <= end)
    return -1;
  if (end > sv->len)
    return -1;

  sv->str += start;
  sv->len = end - start;
  return 0;
}

__HEADER_ONLY int
str_view_substr (struct str_view *src, struct str_view *dst, size_t start,
                 size_t end) {
  return str_view_init_from_substr (dst, src->str, start, end);
}

__HEADER_ONLY int
str_view_cmp (const struct str_view sv1, const struct str_view sv2) {
  if (sv1.len != sv2.len)
    return -1;
  return strncmp (sv1.str, sv2.str, sv1.len);
}

__HEADER_ONLY int
str_view_ncmp (const struct str_view sv1, const struct str_view sv2,
               size_t n) {
  if (n > sv1.len || n > sv2.len)
    return -1;
  return strncmp (sv1.str, sv2.str, n);
}

__HEADER_ONLY int
str_view_icmp (const struct str_view sv1, const struct str_view sv2) {
  if (sv1.len != sv2.len)
    return -1;
  return strnicmp (sv1.str, sv2.str, sv1.len);
}

__HEADER_ONLY int
str_view_nicmp (const struct str_view sv1, const struct str_view sv2,
                size_t n) {
  if (n > sv1.len || n > sv2.len)
    return -1;
  return strnicmp (sv1.str, sv2.str, n);
}

__HEADER_ONLY int
str_view_findc (const struct str_view *sv, const char str) {
  for (size_t i = 0; i < sv->len; i++) {
    if (sv->str[i] == str)
      return i;
  }

  return -1;
}

__HEADER_ONLY int
str_view_findsv (const struct str_view *sv, const struct str_view *sv2) {
  if (sv2->len > sv->len)
    return -1;

  for (size_t i = 0; i < sv->len; i++) {
    if (sv->str[i] == sv2->str[0]) {
      if (str_view_nicmp (*(sv + i), *sv2, sv2->len) == 0)
        return i;
    }
  }

  return -1;
}

// converters
__HEADER_ONLY int
str_view_toi (const struct str_view *sv, int *out) {
  // trim
  struct str_view tmp = *sv;

  // should not use atoi, as we're not sure if the string is null-terminated
  int sign = 1;
  int i = 0;
  if (tmp.str[0] == '-') {
    sign = -1;
    i = 1;
  } else if (tmp.str[0] == '+') {
    i = 1;
  }

  int num = 0;
  // prevent overflow
  for (; i < tmp.len; i++) {
    if (!isdigit (tmp.str[i]))
      return -1;

    int digit = tmp.str[i] - '0';

    if (num > (INT_MAX - num) / 10) {
      *out = (sign == 1) ? INT_MAX : INT_MIN;
    }

    num = num * 10 + digit;
  }

  *out = num * sign;
  return 0;
}

__HEADER_ONLY int
str_view_tol (const struct str_view *sv, long *out) {
  // trim
  struct str_view tmp;
  str_view_init (&tmp, sv->str, sv->len);
  str_view_trim (&tmp);

  // should not use atoi, as we're not sure if the string is null-terminated
  int sign = 1;
  int i = 0;
  if (tmp.str[0] == '-') {
    sign = -1;
    i = 1;
  } else if (tmp.str[0] == '+') {
    i = 1;
  }

  long num = 0;
  // prevent overflow
  for (; i < tmp.len; i++) {
    if (!isdigit (tmp.str[i]))
      return -1;

    int digit = tmp.str[i] - '0';

    if (num > (LONG_MAX - num) / 10) {
      *out = (sign == 1) ? LONG_MAX : LONG_MIN;
    }

    num = num * 10 + digit;
  }

  *out = num * sign;
  return 0;
}

__HEADER_ONLY int
str_view_tod (const struct str_view *sv, double *out) {
  // trim
  struct str_view tmp;
  str_view_init (&tmp, sv->str, sv->len);
  str_view_trim (&tmp);

  int sign = 1;
  double ret = 0;
  int dot = 0;
  int i = 0;

  // check for first char
  if (tmp.str[0] == '-') {
    sign = -1;
    i = 1;
  } else if (tmp.str[0] == '+') {
    i = 1;
  } else if (tmp.str[0] == '.') {
    dot = 1;
    i = 1;
  } else if (!isdigit (tmp.str[0])) {
    return -1;
  }

  for (; i < tmp.len; i++) {
    if (tmp.str[i] == '.') {
      if (dot)
        return -1; // more than one dot

      dot = 1;
      continue;
    }

    if (!isdigit (tmp.str[i]))
      // return -1;
      break;

    int digit = tmp.str[i] - '0';

    if (*out > (INT_MAX - *out) / 10) {
      *out = (sign == 1) ? INT_MAX : INT_MIN;
    }

    if (dot)
      dot++;

    ret = ret * 10 + digit;
  }

  while (dot > 1) {
    ret /= 10;
    dot--;
  }

  *out = ret * sign;
  return i;
}

#if defined(__cplusplus)
}
#endif
