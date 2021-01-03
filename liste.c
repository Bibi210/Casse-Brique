#include "liste.h"

// Alloc on heap a new list.
// It's empty at first.
extern liste_t *list_init(size_t data_size,void free_func(void*))
{
    assert(data_size > 0);
    liste_t *lst = malloc(sizeof(liste_t));

    assert(lst);
    lst->premier = NULL;
    lst->data_size = data_size;
    lst->list_size = 0;
    lst->free_func = free_func;
    return lst;
}

// list to insert at head, data, size of data in byte
extern void list_push(liste_t *l, void *data)
{
    cell_t *nouveau = malloc(sizeof(l->data_size));
    assert(nouveau);

    void *data_copy = malloc(l->data_size);
    assert(data_copy);
    memcpy(data_copy, data, l->data_size);

    nouveau->data = data_copy;
    nouveau->next = l->premier;
    l->premier = nouveau;
    l->list_size++;
}

extern void list_insert_at(liste_t *l, void *data, unsigned long long n)
{
    if (n == 0)
    {
        list_push(l, data);
    }
    else if (n >= l->list_size)
    {
        list_append(l, data);
    }
    else
    {
        cell_t *nouveau = malloc(sizeof(l->data_size));
        assert(nouveau);

        void *data_copy = malloc(l->data_size);
        assert(data_copy);
        memcpy(data_copy, data, l->data_size);

        cell_t *current;
        cell_t *prec;
        for (current = l->premier; n != 0; current = current->next)
        {
            prec = current;
            n--;
        }
        prec->next = nouveau;
        nouveau->data = data_copy;
        nouveau->next = current;
    }
    l->list_size++;
}

void list_append(liste_t *l, void *data)
{
    if (l->list_size == 0)
    {
        list_push(l, data);
    }
    else
    {
        cell_t *nouveau = malloc(sizeof(l->data_size));
        assert(nouveau);

        void *data_copy = malloc(l->data_size);
        assert(data_copy);
        memcpy(data_copy, data, l->data_size);

        nouveau->data = data_copy;
        nouveau->next = NULL;

        cell_t *current;
        for (current = l->premier; current->next != NULL; current = current->next)
        {
        }
        current->next = nouveau;
    }
    l->list_size++;
}
extern void list_printf(liste_t *l, void print_func(void *))
{
    for (cell_t *current = l->premier; current != NULL; current = current->next)
    {
        printf("/****************************/\n");
        print_func(current->data);
    }
    printf("\nFin_liste\n");
}

void list_del_at(liste_t *l, unsigned long long n)
{
    cell_t *current;
    cell_t *prec;
    if (l->list_size <= 0)
    {
        printf("Erreur votre liste ne contient pas d'element");
        return;
    }

    if (n == 0)
    {
        l->free_func(l->premier->data);
        l->premier = l->premier->next;
    }
    else
    {

        for (current = l->premier; n != 0; current = current->next)
        {
            prec = current;
            n--;
        }
        prec->next = current->next;
        l->free_func(current->data);
        free(current);
    }
    l->list_size--;
}

extern void list_free(liste_t *l)
{
    while (l->list_size != 0)
    {
        list_del_at(l, l->list_size - 1);
    }
    free(l->premier);
    free(l);
}
