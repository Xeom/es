#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdlib.h>

#include "clean.h"

struct sigaction sigint_default;
struct sigaction sigterm_default;

static void (*cleanup)(void) = NULL;

static void sigint_handler(int sign)
{
    if (cleanup) cleanup();
    sigaction(sign, &sigint_default, NULL);
    kill(0, sign);
}

static void sigterm_handler(int sign)
{
    if (cleanup) cleanup();
    sigaction(sign, &sigterm_default, NULL);
    kill(0, sign);
}

void on_clean(void (*f)(void))
{
    struct sigaction act;

    cleanup = f;

    atexit(f);

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, &sigint_default);

    act.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &act, &sigterm_default);
}
