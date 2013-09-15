#ifndef __UTIL_H__
#define __UTIL_H__

#include "config.h"

#ifndef HAVE_STRLCAT
extern size_t strlcat(char *dest, const char *src, size_t size);
#endif

extern unsigned char bit_flip(unsigned char x);

#endif /* __UTIL_H__ */

