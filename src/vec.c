typedef struct vec_s vec;

struct vec_s
{
    /* The stored data, can be NULL */
    char *data;
    /* Capacity usage and width in bytes */
    size_t capacity;
    size_t usage;
    size_t width;
};

/* Call after operations that might shorten a vec */
static void vec_resize_shorter(vec *v)
{
    /* Escape if no changes need making */
    if (v->usage >= (v->capacity>> 2)) return;

    /* Decrease the capacity until it's okay */
    do
    {
        v->capacity >>= 1;
    } while (v->usage < (v->capacity >> 2));

    v->data = realloc(v->data, v->capacity);
}

/* Call after operations that might lengthen a vec */
static void vec_resize_longer(vec *v)
{
    /* Escape if no changes need making */
    if (v->usage <= v->capacity) return;

    /* Increase the capacity until the data fits */
    do
    {
        v->capacity <<= 1;
    } while (v->usage > v->capacity);

    v->data  = realloc(v->data, v->capacity);
}

/* Initialises a vector */
void vec_init(vec *v, size_t width)
{
    v->width    = width;
    v->usage    = 0;
    v->capacity = 0;
    v->data     = NULL;
}

/* BISECTION */
size_t vec_bst(
    vec *v,
    const char *item,
    int (*cmpfunc)(const char *a, const char *b)
)
{
    size_t ltind, gtind;

    /* An empty vector returns 0 always */
    if (v->usage == 0) return 0;

    /* ltind and gtind will always be less than and greater than item, *
     * respectively. They are set to the ends of the vec here          */
    ltind = 0;
    gtind = vec_len(v) - 1;

    /* Check that ltind and gtind are less than and greater than item, *
     * otherwise return a limit.                                       */
    if (cmpfunc(vec_get(v, ltind), item) > 0) return 0;
    if (cmpfunc(vec_get(v, gtind), item) < 0) return vec_len(v);

    /* We're done when we've narrowed ltind and gtind down to one apart. *
     * Our new bisection index is between them.                          */
    while (ltind + 1 < gtind)
    {
        size_t midind;
        int cmp;

        midind = (gtind + ltind) / 2;
        cmp = cmpfunc(vec_get(v, midind), item);

        if (cmp  > 0) gtind = midind;
        if (cmp  < 0) ltind = midind;
        if (cmp == 0) return  midind;
    }

    return gtind;
}

/* Get the number of items in a vector */
size_t vec_len(vec *v)
{
    return vec->usage / vec->width;
}

int vec_ins(vec *v, size_t ind, size_t n, const char *data)
{
    size_t bytesafter, offset, bytesins;

    offset     = ind * v->width;
    bytesins   = n * v->width;
    bytesafter = v->usage - offset;

    if (offset > v->usage) return -1;

    v->usage  += bytesins;
    vec_resize_longer(v);

    if (bytesafter)
        memmove(v->data + offset + bytesins, v->data + offset, bytesafter);

    if (data)
        memmove(v->data + offset, data, bytesins);
    else memset(v->data + offset, 0,    bytesins);

    return 0;
}

void vec_del(vec *v, size_t ind, size_t n)
{
    size_t bytesafter, offset;

    offset     = ind * v->width;
    bytesdead  = n * v->width;
    bytesafter = v->usage - offset - bytesdead;

    if (offset + bytesdead > v->usage) return -1;

    if (bytesafter)
        memmove(v->data + offset, v->data + offset + bytesdead, bytesafter);

    v->usage -= n;
    vec_resize_shorter(v);
}

char *vec_get(vec *v, size_t ind)
{
    size_t offset;

    offset = ind * v->width;

    if (offset >= v->width) return NULL;

    return v->data + offset;
}
