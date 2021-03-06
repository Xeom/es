#include <stdio.h>
#include "utf8.h"

uint32_t utf8_get_char(unsigned char utf8)
{
    static uint32_t rtn;
    static size_t   width;
    size_t leading;

    if ((utf8 & 0x80) == 0)
    {
        width = 0;
        rtn   = 0;

        return utf8;
    }

    for (leading = 1; utf8 & (0x80u >> leading); leading++);

    if (leading == 1 && width)
    {
        rtn <<= 6;
        rtn  |= utf8 & 0x3fu;

        width -= 1;

        if (width == 0) return rtn;
    }
    else
    {
        rtn = utf8 & 0x7fu >> leading;
        width = leading - 1;
    }

    return 0;
}

uint32_t utf8_read_char(unsigned char **utf8)
{
    while(**utf8)
    {
        uint32_t chr;

        chr = utf8_get_char(**utf8);
        (*utf8)++;

        if (chr != 0) return chr;
    }

    return 0;
}

int utf8_write_char(uint32_t chr, unsigned char *data)
{
    int width, byte;

    width = 1;

    if (chr <= 0x7f)
        data[0] = (unsigned char)chr;
    else
    {
        for (width = 2; chr >= 1u << (5 * width + 1); width++);
        byte = width;

        while (--byte)
        {
            data[byte] = (unsigned char)(0x80 | (0x3f & chr));
            chr >>= 6;
        }


        data[0]  = (unsigned char)(0xff << (8 - width));
        data[0] |= (unsigned char)chr;
    }

    return width;
}

int utf8_from_utf32(vec *utf8, uint32_t *utf32, size_t n)
{
    size_t chrind;
    int chrwidth;
    unsigned char utf8chr[6];

    for (chrind = 0; chrind < n; chrind++)
    {
        uint32_t chr;
        chr = utf32[chrind];
        chrwidth = utf8_write_char(chr, utf8chr);
        vec_ins(utf8, vec_len(utf8), (size_t)chrwidth, utf8chr);
    }

    return 0;
}

int utf8_to_utf32(unsigned char *utf8, vec *utf32, size_t n)
{
    size_t ind, width;
    uint32_t chr;

    width = 0;

    for (ind = 0; ind < n; ind++)
    {
        unsigned char byte;
        byte = utf8[ind];
        if (width)
        {
            chr <<= 6;
            chr  |= byte & 0x3f;
            --width;
        }
        else
        {
            if ((byte & 0x80) == 0)
                chr = byte;

            else for (width = 1; byte & (0x80 >> width); width++);
        }

        if (width == 0)
            vec_ins(utf32, vec_len(utf32), 1, &chr);

    }

    return 0;
}


