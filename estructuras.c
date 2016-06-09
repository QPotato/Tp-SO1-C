#include "headers/estructuras.h"

Abierto createAbierto(int fd, const char* nombre, mqd_t host)
{
    Abierto new;
    new.fd = fd;
    strcpy(new.nombre, nombre);
    new.host = host;
    return new;
}


void agregarAbiertoEnSesion(Sesion* sesion, int nuevo)
{
    sesion->fd[sesion->nAbiertos] = nuevo;
    sesion->nAbiertos++;
}

void agregarAbiertoEnData(WorkerData* data, Abierto nuevo)
{
    data->abiertos[data->nAbiertos] = nuevo;
    data->nAbiertos++;
}
