#if !defined(VEC_H)
# define VEC_H

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

#endif /* VEC_H */
