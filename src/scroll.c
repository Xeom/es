#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "clean.h"
#include "text.h"
#include "cmd.h"
#include "com.h"

size_t cols = 10;
size_t rows = 10;

size_t scroll_rows = 0;
size_t scroll_cols = 0;

static text buffer;

static void fill_linecont(vec *linecont, size_t ln);
static void handle_ins(size_t ln);
static void handle_del(size_t ln);
static void handle_set(size_t ln);
static void handle_clr(void);
static void update_line(size_t ln);
static void update(void);

static void fill_linecont(vec *linecont, size_t ln)
{
    line *l;
    size_t len;
    uint32_t *str;

    l = text_get_line(&buffer, ln);

    if (l == NULL)
        return;

    len = vec_len(&(l->chars)) - scroll_cols;

    if (cols < len)
        len = cols;

    str = vec_get(&(l->chars), scroll_cols);
    vec_ins(linecont, 0, len, str);
}

static void handle_ins(size_t ln)
{
    text_cmd delcmd;
    text_cmd inscmd;
    size_t outln;

    if (ln > scroll_rows + rows)
        return;

    outln = ln;

    if (outln <= scroll_rows)
        outln  = scroll_rows + 1;

    inscmd.type   = cmd_ins;
    inscmd.lineno = outln - scroll_rows;

    vec_init(&(inscmd.data.linecont), sizeof(uint32_t));
    fill_linecont(&(inscmd.data.linecont), outln);
    cmd_print(&inscmd);
    cmd_kill(&inscmd);

    delcmd.type   = cmd_del;
    delcmd.lineno = rows + 1;

    cmd_print(&delcmd);
    cmd_kill(&delcmd);
}

static void handle_set(size_t ln)
{
    text_cmd setcmd;

    if (ln <= scroll_rows || ln > rows + scroll_rows)
        return;

    setcmd.type = cmd_set;
    setcmd.lineno = ln - scroll_rows;
    vec_init(&(setcmd.data.linecont), sizeof(uint32_t));
    fill_linecont(&(setcmd.data.linecont), ln);
    cmd_print(&setcmd);
    cmd_kill(&setcmd);
}

static void handle_del(size_t ln)
{
    text_cmd delcmd;
    text_cmd inscmd;

    delcmd.type = cmd_del;
    inscmd.type = cmd_ins;

    vec_init(&(inscmd.data.linecont), sizeof(uint32_t));

    if (ln <= scroll_rows)
        delcmd.lineno = 1;

    else if (ln > rows + scroll_rows)
        return;

    else
        delcmd.lineno = ln - scroll_rows;

    cmd_print(&delcmd);
    cmd_kill(&delcmd);

    inscmd.lineno = rows;

    fill_linecont(&(inscmd.data.linecont), rows + scroll_rows);
    cmd_print(&inscmd);
    cmd_kill(&inscmd);
}

static void handle_clr(void)
{
    text_cmd clrcmd;

    clrcmd.type = cmd_clr;

    cmd_print(&clrcmd);
    cmd_kill(&clrcmd);
}

static void update_line(size_t ln)
{
    text_cmd setcmd;

    setcmd.type   = cmd_set;
    setcmd.lineno = ln - scroll_rows;
    vec_init(&(setcmd.data.linecont), sizeof(uint32_t));

    fill_linecont(&(setcmd.data.linecont), ln);

    cmd_print(&setcmd);
    cmd_kill(&setcmd);
}

static void update(void)
{
    size_t ln;

    ln = scroll_rows + 1;

    for (ln = scroll_rows + 1; ln <= scroll_rows + rows; ln++)
        update_line(ln);
}

static void cleanup(void)
{
    com_kill();
    text_kill(&buffer);
}

static void handle_cmd(char *str, size_t n)
{
    text_cmd cmd;

    cmd_decode(&cmd, str);
    cmd_do_text(&buffer, &cmd);

    switch (cmd.type)
    {
    case cmd_ins:handle_ins(cmd.lineno);break;
    case cmd_del:handle_del(cmd.lineno);break;
    case cmd_set:handle_set(cmd.lineno);break;
    case cmd_clr:handle_clr();break;
    }
}

static void handle_dir(char *str, size_t n)
{
    if (strncmp("RESIZE", str, strlen("RESIZE")) == 0)
    {
        sscanf(str, "RESIZE %lu %lu", &cols, &rows);
        update();
    }
    else if (strncmp("SCROLL", str, strlen("SCROLL")) == 0)
    {
        sscanf(str, "SCROLL %lu %lu", &scroll_cols, &scroll_rows);
        update();
    }
}

int main(void)
{
    inp_conf cmd_input = {"cmd", -1, NULL, handle_cmd};
    inp_conf dir_input = {"dir", -1, NULL, handle_dir};

    on_clean(cleanup);

    text_init(&buffer);
    update();

    com_init();
    com_add_input(&cmd_input);
    com_add_input(&dir_input);

    cleanup();
    exit(1);

    while (1)
        com_wait();
}
