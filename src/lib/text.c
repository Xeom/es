#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "text.h"
#include "utf8.h"

static int text_cmd_decode_linecont(text_cmd *cmd, char *encoded);

void line_init(line *l)
{
    vec_init(&(l->chars), sizeof(uint32_t));
}

void line_set_chars(line *l, const char *chars, size_t n)
{
    if (vec_len(&(l->chars)))
        vec_del(&(l->chars), 0, vec_len(&(l->chars)));

    vec_ins(&(l->chars), 0, n, chars);
}

void text_init(text *t)
{
    vec_init(&(t->lines), sizeof(line));
}

line *text_get_line(text *t, size_t ln)
{
    if (ln > text_len(t) || ln == 0)
        return NULL;

    return vec_get(&(t->lines), ln - 1);
}

size_t text_len(text *t)
{
    return vec_len(&(t->lines));
}

int text_ins_line(text *t, size_t ln)
{
    if (ln == 0) return -1;

    if (ln <= text_len(t) + 1)
    {
        vec_ins(&(t->lines), ln - 1, 1, NULL);
        line_init(vec_get(&(t->lines), ln - 1));

        return 0;
    }

    while (ln > text_len(t))
    {
        vec_ins(&(t->lines), text_len(t), 1, NULL);
        line_init(vec_get(&(t->lines), text_len(t) - 1));
    }

    return 0;
}

void text_del_line(text *t, size_t ln)
{
    vec_del(&(t->lines), ln - 1, 1);
}

static int text_cmd_decode_linecont(text_cmd *cmd, char *encoded)
{
    uint32_t chr;
    vec *chrvec;

    chrvec = &(cmd->data.linecont);
    encoded = strchr(encoded, ' ');

    if (encoded == NULL) return -1;

    encoded++;

    vec_init(chrvec, sizeof(uint32_t));

    while ((chr = utf8_read_char((unsigned char **)&encoded)))
    {
        if (chr == '\n') continue;
        vec_ins(chrvec, vec_len(chrvec), 1, &chr);
    }

    return 0;
}

int text_cmd_decode(text_cmd *cmd, char *encoded)
{
    char cmdtype;
    if (sscanf(encoded, "%c%lu", &cmdtype, &(cmd->lineno)) != 2) return -1;

    cmd->type = (int)cmdtype;

    switch (cmd->type)
    {
    case cmd_ins:
    case cmd_set:
        vec_init(&(cmd->data.linecont), sizeof(uint32_t));
        text_cmd_decode_linecont(cmd, encoded);
        break;
    case cmd_col:
        break;
    default:
        break;
    }

    return 0;
}

int text_cmd_encode(text_cmd *cmd, vec *v)
{
    char type, lineno[32];
    vec *chrvec;

    chrvec = &(cmd->data.linecont);

    type = cmd->type;
    vec_ins(v, 0, 1, &type);
    snprintf(lineno, 32, "%lu ", cmd->lineno);
    vec_ins(v, 1, strlen(lineno), lineno);

    switch (cmd->type)
    {
    case cmd_ins:
    case cmd_set:
        utf8_from_utf32(v, vec_get(chrvec, 0), vec_len(chrvec));
    default:
        break;
    }

    return 0;
}

