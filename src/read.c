#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "vec.h"
#include "text.h"
#include "com.h"
#include "clean.h"

void handle_dir(char *str, size_t n);

void read_file(int fd);

void cleanup(void);

void handle_dir(char *str, size_t n)
{
    if (strncmp("READ", str, strlen("READ")) == 0)
    {
        char fname[128];
        int fd;

        sscanf(str, "READ %s", fname);
        fd = open(fname, O_RDONLY);

        if (fd == -1) fprintf(stderr, "Could not open file %s\n", fname);
        else read_file(fd);
    }

}

void read_file(int fd)
{
    size_t lineno;
    vec    buf;
    char   chr;

    vec_init(&buf, 1);
    vec_ins(&buf, 0, 5, "K\nI1 ");

    lineno = 1;

    while (read(fd, &chr, 1) == 1)
    {
        vec_ins(&buf, vec_len(&buf), 1, &chr);

        if (chr == '\n')
        {
            size_t len;

            len = vec_len(&buf);
            write(STDOUT_FILENO, vec_get(&buf, 0), len);
            vec_del(&buf, 0, len);

            vec_ins(&buf, 0, 32, NULL);
            snprintf(vec_get(&buf, 0), 32, "I%lu ", ++lineno);
            len = strlen(vec_get(&buf, 0));
            vec_del(&buf, len, 32 - len);
        }
    }

    fsync(STDOUT_FILENO);
}

void cleanup(void)
{
    com_kill();
}

int main(void)
{
    inp_conf dir_input = {"dir", -1, NULL, handle_dir};

    on_clean(cleanup);

    com_init();
    com_add_input(&dir_input);

    while (1)
    {
        com_wait();
    }

    return 0;
}
