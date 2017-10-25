#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "utf8.h"
#include "ansi.h"
#include "text.h"
#include "com.h"

int cols;
int rows;

int cur_col = 0;
int cur_row = 1;

size_t scroll = 0;

text buffer;

struct sigaction sigint_default;

void  winch_handler(int sign);
void sigint_handler(int sign);


void scroll_lines(int n);
void draw_all(void);
void draw_after(size_t ln);
void draw_line(size_t ln);
void handle_cmd(char *str, size_t n);
void handle_dir(char *str, size_t n);
void cleanup(void);

void winch_handler(int sign)
{
    cols = ansi_cols();
    rows = ansi_rows();
    draw_all();
}

void sigint_handler(int sign)
{
    cleanup();
    sigaction(sign, &sigint_default, NULL);
    kill(0, sign);
}

void draw_all(void)
{
    draw_after(scroll + 1);
}

void draw_after(size_t ln)
{
    size_t maxln, currln;

    maxln = scroll + (size_t)rows;

    if (scroll >= ln)
        currln = scroll + 1;
    else
        currln = ln;

    for (; currln <= maxln; currln++)
        draw_line(currln);
}

void draw_line(size_t ln)
{
    vec    utf8, *chars;
    size_t len;
    long   row;
    line  *l;

    row = (long)ln - (long)scroll - 1;

    ansi_goto(0, (int)row);
    ansi_clear_line();

    if (row < 0) return;

    l = text_get_line(&buffer, (size_t)ln);
    if (l == NULL) return;

    chars = &(l->chars);

    if ((size_t)cols < vec_len(chars))
        len = (size_t)cols;
    else
        len = vec_len(chars);

    vec_init(&utf8, 1);
    utf8_from_utf32(&utf8, vec_get(chars, 0), len);

    fwrite(vec_get(&utf8, 0), 1, vec_len(&utf8), stdout);
}

void handle_cmd(char *str, size_t n)
{
    size_t ln;
    vec *chrs;
    line *l;
    text_cmd cmd;

    text_cmd_decode(&cmd, str);

    chrs = &(cmd.data.linecont);
    ln   = cmd.lineno;

    switch (cmd.type)
    {
    case cmd_set:
        l = text_get_line(&buffer, ln);
        if (l == NULL)
        {
            text_ins_line(&buffer, ln);
            l = text_get_line(&buffer, ln);
        }

        line_set_chars(
            l, vec_get(chrs, 0), vec_len(chrs)
        );

        draw_line(ln);
        break;

    case cmd_ins:
        text_ins_line(&buffer, ln);
        l = text_get_line(&buffer, ln);

        line_set_chars(
            l, vec_get(chrs, 0), vec_len(chrs)
        );

        draw_after(ln);
        break;

    case cmd_del:
        text_del_line(&buffer, ln);
        draw_after(ln);
        break;

    case cmd_clr:
        text_clr(&buffer);
        draw_all();
        break;
    }

    ansi_goto(cur_col, cur_row);
    text_cmd_kill(&cmd);
}

void handle_dir(char *str, size_t n)
{
    if (strncmp("RESIZE", str, strlen("RESIZE")) == 0)
    {
        sscanf(str, "RESIZE %d %d", &cols, &rows);
        draw_all();
    }
    else if (strncmp("CURSOR", str, strlen("CURSOR")) == 0)
    {
        sscanf(str, "CURSOR %d %d", &cur_col, &cur_row);
        ansi_goto(cur_col, cur_row);
    }
    else if (strncmp("SCROLL", str, strlen("SCROLL")) == 0)
    {
        sscanf(str, "SCROLL %lu", &scroll);
        draw_all();
    }
}

void cleanup(void)
{
    text_kill(&buffer);
    com_kill();
}

int main(void)
{
    struct sigaction act;
    inp_conf cmd_input = {"cmd", -1, NULL, handle_cmd};
    inp_conf dir_input = {"dir", -1, NULL, handle_dir};

    atexit(cleanup);

    text_init(&buffer);
    winch_handler(SIGWINCH);

    act.sa_handler = winch_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGWINCH, &act, NULL);

    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, &sigint_default);

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
