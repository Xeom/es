#if _POSIX_C_SOURCE >= 1 || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)
# include <stdio.h>
#else
# define _POSIX_SOURCE
# include <stdio.h>
# undef  _POSIX_SOURCE
#endif

#include <errno.h>
#include <sys/select.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "vec.h"

#include "com.h"

char *com_tmpdir = "/tmp/es";
char  com_prodir[128];

/* Vectors of all inp_conf and out_confs */
vec com_inp;
vec com_out;

static void com_read_set(fd_set *set, int num);
static void com_read_conf(inp_conf *conf);
static int  com_new_dir(char *name);
static int  com_cmp_inp_conf(const void *a, const void *b);
static int  com_cmp_out_conf(const void *a, const void *b);

/* Creates a new directory */
static int com_new_dir(char *name)
{
    struct stat st;

    if (stat(name, &st) == -1)
        mkdir(name, 0700);

    else if (! S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "%s Doesn't look like a directory", name);
        return -1;
    }

    return 0;
}

/* Start up the communication system */
void com_init(void)
{
    /* Start vectors to store out inputs and outputs */
    vec_init(&com_inp, sizeof(inp_conf));
    vec_init(&com_out, sizeof(out_conf));

    com_new_dir(com_tmpdir);
    sprintf(com_prodir, "%s/%d", com_tmpdir, (int)getpid());
    com_new_dir(com_prodir);
}

/* Clean up the communications system */
void com_kill(void)
{
    size_t ind;

    /* For each input ... */
    for (ind = 0; ind < vec_len(&com_inp); ind++)
    {
        inp_conf *conf;

        conf = vec_get(&com_inp, ind);
        com_kill_input(conf->name);
    }

    /* For each output */
    for (ind = 0; ind < vec_len(&com_out); ind++)
    {
        out_conf *conf;

        /* Close the file */
        conf = vec_get(&com_out, ind);
        close(conf->fd);
    }

    if (rmdir(com_prodir) != 0)
        fprintf(stderr, "Couldn't rm dir %s - errno %d\n", com_prodir, errno);

    /* Delete the input and output vectors */
    vec_kill(&com_inp);
    vec_kill(&com_out);
}


/* Compare a config's name to a string. Use with vec_bst */
static int com_cmp_inp_conf(const void *a, const void *b)
{
    return strcmp(((inp_conf *)a)->name, (char *)b);
}
static int com_cmp_out_conf(const void *a, const void *b)
{
    return strcmp(((out_conf *)a)->name, (char *)b);
}

/* Add a new input channel */
void com_add_input(inp_conf *conf)
{
    size_t ind;
    char   fname[128];
    char  *name;

    name = malloc(strlen(conf->name) + 1);
    strcpy(name, conf->name);
    conf->name = name;

    if (conf->fd == -1)
    {
        snprintf(fname, 128, "%s/%s", com_prodir, conf->name);
        if (mkfifo(fname, 0700) != 0)
            fprintf(stderr, "Couldn't make FIFO %s: errno %d\n", fname, errno);

        conf->fd = open(fname, O_RDWR | O_NONBLOCK);

        if (conf->fd == -1)
            fprintf(stderr, "Couldn't open FIFO %s: errno %d\n", fname, errno);
    }

    ind = vec_bst(&com_inp, conf->name, com_cmp_inp_conf);
    vec_ins(&com_inp, ind, 1, conf);
}

void com_kill_input(char *name)
{
    size_t ind;
    inp_conf *conf;
    char   fname[128];

    ind  = vec_bst(&com_inp, name, com_cmp_inp_conf);
    conf = vec_get(&com_inp, ind);
    close(conf->fd);

    vec_del(&com_inp, ind, 1);

    if (unlink(fname) != 0)
        fprintf(stderr, "Couldn't unlink %s: errno %d\n", fname, errno);

    free(conf->name);
}

/* Add a new output channel */
void com_add_output(out_conf *conf)
{
    size_t ind;
    char fname[128];

    ind = vec_bst(&com_out, conf->name, com_cmp_out_conf);
    vec_ins(&com_out, ind, 1, conf);

    sprintf(fname, "%s/%d/%s", com_tmpdir, conf->pid, conf->name);

    conf->fd = open(fname, O_WRONLY | O_NONBLOCK);
}

static int com_fill_set(fd_set *set)
{
    size_t ind;
    int maxfd;

    FD_ZERO(set);
    maxfd = 0;

    for (ind = 0; ind < vec_len(&com_inp); ind++)
    {
        inp_conf *conf;

        conf = vec_get(&com_inp, ind);
        FD_SET(conf->fd, set);

        if (conf->fd > maxfd) maxfd = conf->fd;
    }

    return maxfd;
}

static void com_read_conf(inp_conf *conf)
{
    if (conf->funct_fd)(conf->funct_fd)(conf->fd);

    if (conf->funct_str)
    {
        vec  str;
        char chr;

        vec_init(&str, 1);

        while (read(conf->fd, &chr, 1) == 1 && chr != '\n')
            vec_ins(&str, vec_len(&str), 1, &chr);

        vec_ins(&str, vec_len(&str), 1, "\0");

        (conf->funct_str)(vec_get(&str, 0), vec_len(&str));

        vec_kill(&str);
    }
}

static void com_read_set(fd_set *set, int num)
{
    size_t ind;

    for (ind = 0; ind < vec_len(&com_inp); ind++)
    {
        inp_conf *conf;

        conf = vec_get(&com_inp, ind);

        if (FD_ISSET(conf->fd, set))
        {
            com_read_conf(conf);

            if (!(--num)) break;
        }
    }
}

void com_send(char *name, char *str)
{
    size_t    ind;
    out_conf *conf;

    ind  = vec_bst(&com_out, name, com_cmp_out_conf);
    conf = vec_get(&com_out, ind);

    if (strcmp(name, conf->name) != 0)
    {
        fprintf(stderr, "Output channel '%s' doesn't exist\n", name);
        return;
    }

    write(conf->fd, str, strlen(str));
}

void com_wait(void)
{
    fd_set set;
    int maxfd, numfds;

    maxfd = com_fill_set(&set);

    numfds = select(maxfd + 1, &set, NULL, NULL, NULL);

    com_read_set(&set, numfds);
}

