#if !defined(UTF8_H)
# define UTF8_H

#include <stdint.h>

#include "vec.h"

uint32_t utf8_get_char(unsigned char utf8);
uint32_t utf8_read_char(unsigned char **utf8);
int utf8_write_char(uint32_t chr, unsigned char *data);
int utf8_from_utf32(vec *utf8, uint32_t *utf32, size_t n);
int utf8_to_utf32(unsigned char *utf8, vec *utf32, size_t n);

#endif /* UTF8_H */
