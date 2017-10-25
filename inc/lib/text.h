#if !defined(TEXT_H)
# define TEXT_H
# include "vec.h"

typedef struct line_s line;
typedef struct text_s text;
typedef struct text_cmd_s text_cmd;

struct text_s
{
    vec lines;
};

struct line_s
{
    vec chars;
    vec colours;
};

typedef enum
{
    cmd_ins = 'I',
    cmd_set = 'S',
    cmd_del = 'D',
    cmd_col = 'C',
    cmd_clr = 'K'
} text_cmd_type;

struct text_cmd_s
{
    text_cmd_type type;
    size_t lineno;

    union
    {
        vec linecont;

        struct
        {
            char NO;
        } col;
    } data;
};

void line_init(line *l);

void line_set_chars(line *l, const char *chars, size_t n);

void text_init(text *t);

void text_kill(text *t);

void text_clr(text *t);

line *text_get_line(text *t, size_t ln);

size_t text_len(text *t);

int text_ins_line(text *t, size_t ln);

void text_del_line(text *t, size_t ln);

int text_cmd_decode(text_cmd *cmd, char *encoded);

void text_cmd_kill(text_cmd *cmd);

int text_cmd_encode(text_cmd *cmd, vec *v);

#endif /* TEXT_H */
