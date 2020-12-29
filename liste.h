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

    unsigned long long list_size;
} liste_t;

extern liste_t *list_init(size_t data_size);
/// Insert in list a data, size registered at list creation.
extern void list_push_front(liste_t *l, void *data);
extern void list_print(liste_t *l, void print_func(void *));
extern void list_insert_at(liste_t *l, void *data, unsigned long long n);
extern void list_append(liste_t *l, void *data);
extern void list_free(liste_t *l);
extern void list_del_at(liste_t *l, unsigned long long n);