#if !defined(CMD_H)
# define CMD_H
# include "vec.h"
# include "text.h"
# include <stdint.h>

typedef struct text_cmd_s text_cmd;

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

int cmd_decode(text_cmd *cmd, char *encoded);

void cmd_kill(text_cmd *cmd);

int cmd_encode(text_cmd *cmd, vec *v);

void cmd_do_text(text *t, text_cmd *cmd);

void cmd_print(text_cmd *cmd);

#endif /* CMD_H */
