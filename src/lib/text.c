
#include "text.h"
#include "utf8.h"

void line_init(line *l)
{
    vec_init(&(l->chars), sizeof(uint32_t));
}

void line_set_chars(line *l, const uint32_t *chars, size_t n)
{
    if (vec_len(&(l->chars)))
        vec_del(&(l->chars), 0, vec_len(&(l->chars)));

    vec_ins(&(l->chars), 0, n, chars);
}

void text_init(text *t)
{
    vec_init(&(t->lines), sizeof(line));
}

void text_kill(text *t)
{
    size_t ind;

    for (ind = 0; ind < vec_len(&(t->lines)); ind++)
        vec_kill(vec_get(&(t->lines), ind));

    vec_kill(&(t->lines));
}

void text_clr(text *t)
{
    vec_del(&(t->lines), 0, vec_len(&(t->lines)));
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
