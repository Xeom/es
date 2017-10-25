#if !defined(ANSI_H)
# define ANSI_H
# include <stdio.h>
# include "text.h"

int ansi_rows(void);

int ansi_cols(void);

void ansi_goto(int x, int y);

void ansi_clear(void);

void ansi_clear_line(void);

#endif /* ANSI_H */
