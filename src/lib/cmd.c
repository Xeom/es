#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "cmd.h"
#include "utf8.h"

static int cmd_decode_linecont(text_cmd *cmd, char *encoded);

static int cmd_decode_linecont(text_cmd *cmd, char *encoded)
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

int cmd_decode(text_cmd *cmd, char *encoded)
{
    char cmdtype;

    cmdtype = *encoded;

    cmd->type = (int)cmdtype;

    if (cmd->type == cmd_clr) return 0;

    if (sscanf(encoded, "%c%lu", &cmdtype, &(cmd->lineno)) != 2) return -1;

    switch (cmd->type)
    {
    case cmd_ins:
    case cmd_set:
        vec_init(&(cmd->data.linecont), sizeof(uint32_t));
        cmd_decode_linecont(cmd, encoded);
        break;
    case cmd_col:
        break;
    default:
        break;
    }

    return 0;
}

void cmd_kill(text_cmd *cmd)
{
    switch (cmd->type)
    {
    case cmd_ins:
    case cmd_set:
        vec_kill(&(cmd->data.linecont));
        break;
    }

    cmd->type = 0;
}

int cmd_encode(text_cmd *cmd, vec *v)
{
    char type, lineno[32];
    vec *chrvec;

    chrvec = &(cmd->data.linecont);

    type = cmd->type;
    vec_ins(v, 0, 1, &type);

    switch (cmd->type)
    {
    case cmd_del:
        snprintf(lineno, 32, "%lu ", cmd->lineno);
        vec_ins(v, 1, strlen(lineno), lineno);
        break;
    case cmd_ins:
    case cmd_set:
        snprintf(lineno, 32, "%lu ", cmd->lineno);
        vec_ins(v, 1, strlen(lineno), lineno);
        utf8_from_utf32(v, vec_get(chrvec, 0), vec_len(chrvec));
    default:
        break;
    }

    return 0;
}

void cmd_do_text(text *t, text_cmd *cmd)
{
    size_t ln;
    vec  *chrs;
    line *l;

    chrs = &(cmd->data.linecont);
    ln   = cmd->lineno;

    switch (cmd->type)
    {
    case cmd_set:
        l = text_get_line(t, ln);
        if (l == NULL)
        {
            text_ins_line(t, ln);
            l = text_get_line(t, ln);
        }

        line_set_chars(
            l, vec_get(chrs, 0), vec_len(chrs)
        );
        break;

    case cmd_ins:
        text_ins_line(t, ln);
        l = text_get_line(t, ln);

        line_set_chars(
            l, vec_get(chrs, 0), vec_len(chrs)
        );
        break;

    case cmd_del:
        text_del_line(t, ln);
        break;

    case cmd_clr:
        text_clr(t);
        break;
    }
}

void cmd_print(text_cmd *cmd)
{
    vec chrs;

    vec_init(&chrs, 1);
    cmd_encode(cmd, &chrs);
    write(STDOUT_FILENO, vec_get(&chrs, 0), vec_len(&chrs));
    write(STDOUT_FILENO, "\n", 1);
    vec_kill(&chrs);
}
