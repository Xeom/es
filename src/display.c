#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "clean.h"
#include "utf8.h"
#include "text.h"
#include "com.h"
#include "cmd.h"

int cols = 10;
int rows = 10;

int cur_col = 0;
int cur_row = 1;

size_t scroll_cols = 0;
size_t scroll_rows = 0;

text buffer;

static void ansi_goto(int x, int y);
static void ansi_clear(void);
static void ansi_clear_line(void);
static void update_all(void);
static void update_after(size_t ln);
static void update_line(size_t ln);
static void draw_line(vec *utf32);
static void handle_cmd(char *str, size_t n);
static void handle_dir(char *str, size_t n);
static void cleanup(void);

static void ansi_goto(int x, int y)
{
    printf("\033[%d;%df", y + 1, x + 1);
}

static void ansi_clear(void)
{
    printf("\033[2J");
}

static void ansi_clear_line(void)
{
    printf("\033[K");
}

static void update_all(void)
{
    update_after(scroll_rows + 1);
}

static void update_after(size_t ln)
{
    size_t maxln, currln;

    maxln = scroll_rows + (size_t)rows;

    if (scroll_rows >= ln)
        currln = scroll_rows + 1;
    else
        currln = ln;

    for (; currln <= maxln; currln++)
        update_line(currln);
}

static void update_line(size_t ln)
{
    long   row;
    line  *l;

    row = (long)ln - (long)scroll_rows - 1;

    ansi_goto(0, (int)row);
    ansi_clear_line();

    if (row < 0) return;

    l = text_get_line(&buffer, (size_t)ln);
    if (l == NULL) return;

    draw_line(&(l->chars));
}

static void draw_line(vec *utf32)
{
    vec    utf8;
    size_t len;

    if (scroll_cols >= vec_len(utf32))
        return;

    len = vec_len(utf32) - scroll_cols;

    if ((size_t)cols < len)
        len = (size_t)cols;

    vec_init(&utf8, 1);
    utf8_from_utf32(&utf8, vec_get(utf32, scroll_cols), len);

    fwrite(vec_get(&utf8, 0), 1, vec_len(&utf8), stdout);
    fflush(stdout);

    vec_kill(&utf8);
}

static void handle_cmd(char *str, size_t n)
{
    size_t ln;
    text_cmd cmd;

    cmd_decode(&cmd, str);
    cmd_do_text(&buffer, &cmd);
    ln   = cmd.lineno;

    switch (cmd.type)
    {
    case cmd_set:
        update_line(ln);
        break;
    case cmd_del:
    case cmd_ins:
        update_after(ln);
        break;
    case cmd_clr:
        update_all();
        break;
    }

    ansi_goto(cur_col, cur_row);
    cmd_kill(&cmd);
}

static void handle_dir(char *str, size_t n)
{
    if (strncmp("RESIZE", str, strlen("RESIZE")) == 0)
    {
        sscanf(str, "RESIZE %d %d", &cols, &rows);
        update_all();
    }
    else if (strncmp("CURSOR", str, strlen("CURSOR")) == 0)
    {
        sscanf(str, "CURSOR %d %d", &cur_col, &cur_row);
        ansi_goto(cur_col, cur_row);
    }
    else if (strncmp("SCROLL", str, strlen("SCROLL")) == 0)
    {
        sscanf(str, "SCROLL %lu %lu", &scroll_cols, &scroll_rows);
        update_all();
    }
}

static void cleanup(void)
{
    text_kill(&buffer);
    com_kill();
    ansi_clear();
}

int main(void)
{
    inp_conf cmd_input = {"cmd", -1, NULL, handle_cmd};
    inp_conf dir_input = {"dir", -1, NULL, handle_dir};

    on_clean(cleanup);

    text_init(&buffer);

    update_all();

    com_init();
    com_add_input(&cmd_input);
    com_add_input(&dir_input);

    while (1)
    {
        fflush(stdout);
        com_wait();
    }

    return 0;
}
