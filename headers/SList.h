#ifndef __SLIST_H__
#define __SLIST_H__

typedef void (*VisitorFuncInt) (void* data, void *extra_data);
typedef int (*CompFunc)(void *a, void *b);
/**
 * Los campos son privados, y no deberian ser accedidos
 * desde el código cliente.
 */
typedef struct _SList {
        void*    data;
        struct _SList *next;
} SList;

#define slist_data(l)       (l)->data
#define slist_next(l)       (l)->next

/**
 * Agrega un elemento al final de la lista, en complejidad O(n).
 *
 * Nota: una lista vacia se representa con un (SList *) NULL.
 */
SList *slist_append(SList* list, void* data);

/**
 * Agrega un elemento al inicio de la lista, en complejidad O(1).
 *
 * Nota: una lista vacia se representa con un (SList *) NULL.
 */
SList *slist_prepend(SList* list, void* data);

/**
 * Destruccion de la lista.
 */
void  slist_destroy(SList* list);

/**
 * Recorrido de la lista, utilizando la funcion pasada.
 */
void  slist_foreach(SList* list, VisitorFuncInt visit, void* extra_data);

//agregadas:
int     slist_has_next(SList* lista);
int     slist_lenght(SList* lista);
SList*  slist_concat(SList* lista1, SList *lista2);
SList*  slist_insert(SList* lista, unsigned int pos, void* n);
SList*  slist_remove(SList* lista, void* n, CompFunc comp);
int     slist_contains(SList* lista, void* n, CompFunc comp);
int slist_index(SList* lista, void* n, CompFunc comp);
void* slist_nth(Slist* lista, int n);
SList*  slist_intersect(SList* lista1, SList* lista2, CompFunc comp);
SList*  slist_sort(SList* lista, CompFunc comp);
#endif /* __SLIST_H__ */
