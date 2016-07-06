#include "headers/estructuras.h"
#include "headers/auxiliares.h"
#include <unistd.h>
#include "headers/SList.h"

// Crea un abierto nuevo con los parametros dados.
Abierto createAbierto(int fd, const char* nombre, mqd_t host)
{
    Abierto new;
    new.fd = fd;
    strcpy(new.nombre, nombre);
    new.host = host;
    return new;
}

// Agrega un FD a la lista de la sesion.
void agregarAbiertoEnSesion(Sesion* sesion, int nuevo)
{
    sesion->fd[sesion->nAbiertos] = nuevo;
    sesion->nAbiertos++;
}

// Devuelve 1 si la sesion abrio un archivo con ese FD.
int sesionTieneFD(Sesion* sesion, int FD)
{
    for(int i = 0; i< sesion->nAbiertos; i++)
        if(sesion->fd[i] == FD)
            return 1;
    return 0; 
}

// Borra un FD de la lista de la sesion y defragmenta.
void cerrarEnSesion(Sesion* sesion, int FD)
{
    for(int i = 0; i< sesion->nAbiertos; i++)
        if(sesion->fd[i] == FD)
        {
            for(int j = i; j < sesion->nAbiertos - 1; j++)
                sesion->fd[j] = sesion->fd[j + 1];
            break;
        }
    sesion->nAbiertos--;
}

// Agrega un abierto a la lista de la data
void agregarAbiertoEnData(WorkerData* data, Abierto nuevo)
{
    data->abiertos[data->nAbiertos] = nuevo;
    data->nAbiertos++;
}

//Devuelve 1 si hay un abierto de nombre nombreAr en abiertos.
int estaAbierto(WorkerData* data, const char* nombreAr)
{
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    for(int i = 0; i < nAbiertos; i++)
        if(strcmp(nombreAr, abiertos[i].nombre) == 0)
            return 1;
    return 0;
}

//Devuelve 1 si hay un abierto con ese FD en abiertos.
int esLocalFD(WorkerData* data, int FD)
{
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    for(int i = 0; i < nAbiertos; i++)
        if(abiertos[i].fd == FD)
            return 1;
    return 0;
}

// Borra un FD de la lista de abiertos y defragmenta.
void cerrarEnData(WorkerData* data, int FD)
{
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    if(close(FD) != 0)
    {
        printf("Flashiamos en cerrarEnData (con FD = %d)\n", FD);
    }
    for(int i = 0; i < nAbiertos; i++)
        if(abiertos[i].fd == FD)
        {
            for(int j = i; j < data->nAbiertos - 1; j++)
                abiertos[j] = abiertos[j + 1];
            break;
        }
    data->nAbiertos--;
}

void cerrarSesion(WorkerData* data, int id)
{
    SList *sesiones = data->sesiones;
    slist_remove(sesiones, slist_nth(sesiones, id), sesion_comp);
}

