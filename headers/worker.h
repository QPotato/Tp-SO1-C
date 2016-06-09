//Estructuras y constantes que necesitan los workers.
#ifndef __WORKER_H__
#define __WORKER_H__

#define N_WORKERS 6

#define HELP_OPN_NOTFOUND -1
#define HELP_OPN_INUSE -2

#define HELP_DEL_OK 0
#define HELP_DEL_NOTFOUND -1
#define HELP_DEL_INUSE -2

#include "estructuras.h"
#include "SList.h"
#include "mensajes.h"
#include "procesos.h"

//Funciones Auxiliares
int mqd_t_comp(void* a, void* b);

void getFiles(int id, mqd_t *workers, char *nombres);

int isConnected(mqd_t cumpa, SList* sesiones);

//Handles de comandos
void handleCON(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleLSD(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleCRE(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleDEL(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleOPN(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleREA(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleWRT(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleCLO(ParametrosWorker params, WorkerData *data, Msg *msg);

void handleBYE(ParametrosWorker params, WorkerData *data, Msg *msg);


//Handles de pedidos de ayuda
void helpLSD(ParametrosWorker params, WorkerData *data, Msg *msg);

void helpDEL(ParametrosWorker params, WorkerData *data, Msg *msg);

void helpOPN(ParametrosWorker params, WorkerData *data, Msg *msg);

void helpWRT(ParametrosWorker params, WorkerData *data, Msg *msg);

void helpREA(ParametrosWorker params, WorkerData *data, Msg *msg);

void helpCLO(ParametrosWorker params, WorkerData *data, Msg *msg);



#endif // __WORKER_H__
