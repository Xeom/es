#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#include "utf8.h"

#include "ansi.h"

int ansi_rows(void)
{
    struct winsize w;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

    return w.ws_row;
}

int ansi_cols(void)
{
    struct winsize w;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

void ansi_goto(int x, int y)
{
    printf("\033[%d;%df", y + 1, x + 1);
}

void ansi_clear(void)
{
    printf("\033[2J");
}

void ansi_clear_line(void)
{
    printf("\033[K");
}



