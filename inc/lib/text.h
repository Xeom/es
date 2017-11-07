#if !defined(TEXT_H)
# define TEXT_H
# include "vec.h"
# include <stdint.h>

typedef struct line_s line;
typedef struct text_s text;

struct text_s
{
    vec lines;
};

struct line_s
{
    vec chars;
    vec colours;
};


void line_init(line *l);

void line_set_chars(line *l, const uint32_t *chars, size_t n);

void text_init(text *t);

void text_kill(text *t);

void text_clr(text *t);

line *text_get_line(text *t, size_t ln);

size_t text_len(text *t);

int text_ins_line(text *t, size_t ln);

void text_del_line(text *t, size_t ln);

#endif /* TEXT_H */
