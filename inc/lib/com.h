#if !defined(COM_H)
# define COM_H
# include <stdio.h>

/* Structures for storing data about input and output channels */
typedef struct inp_conf_s inp_conf;
typedef struct out_conf_s out_conf;

struct inp_conf_s
{
    char *name; /* The name of the channel */
    int   fd;   /* A file pointer to the channel stream */

    /* If not null, functinos called when file is readable */
    void (*funct_fd)(int fd);
    void (*funct_str)(char *str, size_t n);
};

struct out_conf_s
{
    char *name; /* The name of the channel */
    int   pid;  /* The PID of the process we're writing to */
    int   fd;   /* A file pointer to the channel stream */
};

void com_init(void);

void com_add_input(inp_conf *conf);

void com_add_output(out_conf *conf);

void com_kill_input(char *name);

void com_send(char *name, char *str);

void com_wait(void);

void com_kill(void);

#endif /* COM_H */
