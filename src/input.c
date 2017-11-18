#define _POSIX_C_SOURCE 1
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "utf8.h"

/* The escape key */
#define K_ESC 0x1b

typedef struct escconf_s escconf;

struct escconf_s
{
    char *name;
    char *combo;
};

/* An array of escape sequences translated to key names */
escconf escapesarray[] = {
    {"UP",     "[A"},
    {"DOWN",   "[B"},
    {"RIGHT",  "[C"},
    {"LEFT",   "[D"},
    {"HOME",   "[1~"},
    {"DEL",    "[3~"},
    {"END",    "[4~"},
    {"PGUP",   "[5~"},
    {"PGDOWN", "[6~"},
    {"INSERT", "[4h"},
    {"F1",     "OP"},
    {"F2",     "OQ"},
    {"F3",     "OR"},
    {"F4",     "OS"},
    {"F5",     "[15~"},
    {"F6",     "[17~"},
    {"F7",     "[18~"},
    {"F8",     "[19~"},
    {"F9",     "[20~"},
    {"F10",    "[21~"},
    {"F11",    "[23~"},
    {"F12",    "[24~"}
};

vec escapes;

/* Keep a copy of the default sigint action to call after a custom action */
struct sigaction sigint_default;

/* Functions to compare escconfs for sorting - escconf_cmp_str compares *
 * a escconf to a string, and escconf_cmp compares two escconfs         */
int escconf_cmp_str(const void *a, const void *str);
int escconf_cmp(const void *a, const void *b);

/* Signal handlers */
void  winch_handler(int sign);
void sigint_handler(int sign);

/* Handler for when a key is typed, and function to print out key names, *
 * both stateful.                                                        */
void key_handler(int chr);
void key_print(uint32_t k);

void cleanup(void);

/* Functions to initialize things */
void escapes_init(void);
void term_init(void);

int escconf_cmp(const void *a, const void *b)
{
    return strcmp(((const escconf *)a)->combo, ((const escconf *)b)->combo);
}

int escconf_cmp_str(const void *a, const void *str)
{
    return strcmp(((const escconf *)a)->combo, (char *)str);
}

void winch_handler(int sign)
{
    struct winsize w;

    /* Get and print the new stdin size. We use stdin not stdout since *
     * this program faces the user only on stdin.                      */
    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
    printf("RESIZE %d %d\n", w.ws_col, w.ws_row);
    fflush(stdout);
}

void sigint_handler(int sign)
{
    /* Clean up the re-mount the default action */
    cleanup();
    sigaction(sign, &sigint_default, NULL);
    kill(0, sign);
}

void key_handler(int chr)
{
    uint32_t rtn;

    /* Iterate stateful get_char function */
    rtn = utf8_get_char((unsigned char)chr);

    /* If we complete a utf keypress, print it out */
    if (rtn) key_print(rtn);
}

void key_print(uint32_t k)
{
    static vec esccombo = { .data = NULL };
    static int escaped = 0;

    if (esccombo.data == NULL)
    {
        vec_init(&esccombo, 1);
        vec_ins(&esccombo, 0, 1, "\0");
    }

    if (k == K_ESC)
    {
        escaped = 1;
        vec_del(&esccombo, 0, vec_len(&esccombo) - 1);
    }

    else if (escaped == 1)
    {
        unsigned char kchr;
        size_t ind, clen;
        escconf *match;
        char *cstr;

        kchr = (unsigned char)k;
        clen = vec_len(&esccombo);

        vec_ins(&esccombo, clen - 1, 1, &kchr);
        cstr = vec_get(&esccombo, 0);
        ind = vec_bst(&escapes, cstr, escconf_cmp_str);

        match = vec_get(&escapes, ind);

        if (ind >= vec_len(&escapes) ||
            strncmp(match->combo, cstr, clen) != 0)
        {
            kchr = (unsigned char)cstr[0];
            printf("KEY E_%x\n", (int)kchr);

            for (ind = 1; ind < clen; ind++)
            {
                kchr = (unsigned char)cstr[ind];
                printf("KEY K_%x\n", (int)kchr);
            }

            fflush(stdout);
            escaped = 0;

            return;
        }

        if (0 == strcmp(match->combo, cstr))
        {
            printf("KEY K_%s\n", match->name);
            vec_del(&esccombo, 0, vec_len(&esccombo) - 1);

            fflush(stdout);
            escaped = 0;
        }
    }
    else
    {
        printf("KEY K_%x\n", k);
        fflush(stdout);
    }
}

void cleanup(void)
{
}

void escapes_init(void)
{
    qsort(
        escapesarray,
        sizeof(escapesarray) / sizeof(escconf),
        sizeof(escconf),
        escconf_cmp
    );

    vec_init(&escapes, sizeof(escconf));
    vec_ins(&escapes, 0, sizeof(escapesarray) / sizeof(escconf), escapesarray);
}

void term_init(void)
{
    struct termios   tinfo;

    tcgetattr(STDIN_FILENO, &tinfo);

    tinfo.c_lflag -= tinfo.c_lflag & ICANON;
    tinfo.c_lflag -= tinfo.c_lflag & ECHO;

    tinfo.c_cc[VMIN] = 1;
    tinfo.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &tinfo);
}

int main(void)
{
    struct sigaction act;

    escapes_init();
    term_init();

    winch_handler(SIGWINCH);

    act.sa_handler = winch_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGWINCH, &act, NULL);

    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, &sigint_default);

    while (1)
         key_handler(getchar());

    return 0;
}
