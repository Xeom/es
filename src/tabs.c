#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <string.h>
#include <stdint.h>

#include "clean.h"
#include "cmd.h"
#include "com.h"

unsigned tabwidth = 4;

static void handle_cmd(char *str, size_t n);
static void cleanup(void);
static void spaceify(text_cmd *cmd);

static void spaceify(text_cmd *cmd)
{
    size_t ind;
    vec *chrs;

    chrs = &(cmd->data.linecont);

    for (ind = 0; ind < vec_len(chrs); ind++)
    {
        uint32_t *str;
        str = vec_get(chrs, ind);
        if (*str == '\t')
        {
            size_t spaces;
            spaces = (size_t)tabwidth - ind % (size_t)tabwidth;
            vec_ins(chrs, ind, spaces - 1, NULL);
            ind += spaces - 1;
            while (spaces--) str[spaces] = ' ';
        }
    }
}

static void handle_cmd(char *str, size_t n)
{
    text_cmd cmd;

    cmd_decode(&cmd, str);

    if (cmd.type == cmd_ins || cmd.type == cmd_set)
        spaceify(&cmd);

    cmd_print(&cmd);
    cmd_kill(&cmd);
}

static void cleanup(void)
{
    com_kill();
}

int main(void)
{
    inp_conf cmd_input = {"cmd", -1, NULL, handle_cmd};

    on_clean(cleanup);

    com_init();
    com_add_input(&cmd_input);

    while (1)
        com_wait();

    return 0;

}
