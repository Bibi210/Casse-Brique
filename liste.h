#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct cell_s
{
    void *data;
    struct cell_s *next;
} cell_t;

typedef struct
{
    // Begin of the list
    cell_t *premier;
    size_t data_size;
    void (*free_func)(void*);
    unsigned long long list_size;
} liste_t;

extern liste_t *list_init(size_t data_size,void free_func(void*));
/// Insert in list a data, size registered at list creation.
extern void list_push(liste_t *, void *);
extern void list_printf(liste_t *, void print_func(void *));
extern void list_insert_at(liste_t *, void *, unsigned long long);
extern void list_append(liste_t *, void *);
extern void list_free(liste_t *);
extern void list_del_at(liste_t *, unsigned long long);