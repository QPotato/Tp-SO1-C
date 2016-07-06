#include <stdlib.h>
#include <stdio.h>
#include "headers/SList.h"

SList* slist_append(SList* list, void* data)
{
      SList *newNode = malloc(sizeof(SList));
      SList *node;
      slist_data(newNode) = data;
      slist_next(newNode) = NULL;
      if (list == NULL) {
         return newNode;
      }
      node = list;
      while (slist_next(node) != NULL) {
            node = slist_next(node);
      }
      /* ahora 'node' apunta al ultimo nodo en la lista */
      slist_next(node) = newNode;
      return list;
}

SList* slist_prepend(SList *list, void* data)
{
      SList *newNode = malloc(sizeof(SList));
      slist_next(newNode) = list;
      slist_data(newNode) = data;
      return newNode;
}

void  slist_destroy(SList *list)
{
      SList *nodeToDelete;

      while (list != NULL) {
            nodeToDelete = list;
            list = slist_next(list);
            free(nodeToDelete->data);
            free(nodeToDelete);
      }
}

void  slist_foreach(SList *list, VisitorFuncInt visit, void* extra_data)
{
      SList *node = list;

      while (node != NULL) {
            visit(slist_data(node), extra_data);
            node = slist_next(node);
      }
}
int slist_has_next(SList* lista){
    if(lista->next!=NULL)
        return 1;
    else
        return 0;
}
int slist_lenght(SList* lista){
    int t=1;
    if(lista==NULL)
        return 0;
    while(slist_has_next(lista)){
        t++;
        lista=slist_next(lista);
    }
    return t;
}
SList*  slist_concat(SList* lista1, SList *lista2){
    SList* conc=NULL;
    int i;
    for(i=0;i<slist_lenght(lista1);i++){
        slist_append(conc,lista1->data);
        lista1=slist_next(lista1);
    }
    for(i=0;i<slist_lenght(lista2);i++){
        slist_append(conc,lista2->data);
        lista2=slist_next(lista2);
    }
    return conc;
}
SList*  slist_insert(SList* lista,unsigned int pos, void* n){
    int i;
    SList* l=lista;
    for(i=0;i<pos;i++)
        l=slist_next(l);
    SList *nodo=malloc(sizeof(SList));
    nodo->data=n;
    nodo->next=slist_next(l);
    l->next=nodo;
    return lista;
}

SList* slist_remove(SList* lista, void* dato, CompFunc comp)
{
    SList *l = lista;
    if(comp(l->data, dato) == 0)
    {
        l = l->next;
        return lista;
    }
    else
    {
        while(l->next != NULL && comp(l->next->data, dato) != 0)
            l = l->next;
        l->next = l->next->next;
        return lista;
    }
}

int     slist_contains(SList* lista, void* n, CompFunc comp){
    SList* l;
    for(l=lista;l!=NULL;l=l->next)
        if(comp(l->data,n)==0)
            return 1;
    return 0;
}
int slist_index(SList* lista, void* n, CompFunc comp){
    SList* l;
    int i=0;
    for(l=lista;l!=NULL;l=l->next,i++)
        if(comp(l->data,n)==0)
            return i;
    return -1;        
}
SList*  slist_intersect(SList* lista1, SList* lista2, CompFunc comp){
    SList *intersec=NULL;
    for(;lista1!=NULL;lista1=lista1->next)
        for(;lista2!=NULL;lista2->next)
            if(comp(lista1->data,lista2->data)==0 && !slist_contains(intersec,lista1->data, comp))
                slist_append(intersec,lista1->data);
    return intersec;
}
void swap(void** a, void** b){
    void* c=*a;
    *a=*b;
    *b=c;
}
SList*  slist_sort(SList* lista, CompFunc comp){
    SList *l, *k;
    if(lista==NULL)
        return 0;
    for(l=lista;l!=NULL;l=l->next)
        for(k=lista;k!=l;k=k->next){
            if(comp(k->data,k->next->data)<0){
                swap(&k->data,&k->next->data);
            }
        }
    return lista;
}

void* slist_nth(SList* lista, int n)
{
    while(n > 0)
    {
        if(lista->next == NULL)
        {
            printf("Acceso a SList fuera de rango\n");
            return NULL;
        }
        lista = lista->next;
        n--;
    }
    return lista->data;
}
