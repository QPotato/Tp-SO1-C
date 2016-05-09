//Estructuras y constantes que necesitan los workers.
#ifndef __WORKER_H__
#define __WORKER_H__

#define N_WORKERS 6
#define MAX_NOMBRE 128
#define MAX_ARCHIVOS 1024

#include "estructuras.h"
#include "SList.h"
#include "mensajes.h"


typedef struct
{
    int id;
    mqd_t casilla;
    SList* abiertos;
} Sesion;

typedef struct
{
    SList* sesiones;
    int maxIDlocal;
} WorkerData;
#endif // __WORKER_H__
