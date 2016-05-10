//Estructuras y constantes que necesitan los workers.
#ifndef __WORKER_H__
#define __WORKER_H__

#define N_WORKERS 6
#define MAX_ARCHIVOS 1024

#include "estructuras.h"
#include "SList.h"
#include "mensajes.h"
#include "procesos.h"


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

//Funciones Auxiliares
int mqd_t_comp(void* a, void* b);

void getFiles(int id, mqd_t *workers, char *nombres);

int isConnected(mqd_t cumpa, SList* sesiones);

//Handles de comandos
void handleCON(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleLSD(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleCRE(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleDEL(ParametrosWorker params, WorkerData *data, Msg *msg);

//Handles de pedidos de ayuda
void helpLSD(ParametrosWorker params, WorkerData *data, Msg *msg);

#endif // __WORKER_H__
