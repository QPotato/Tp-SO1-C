//Define estructuras que necesitan los modulos que van spawnear procesos, los que van a ser spawneados
#ifndef __PROCESOS_H__
#define __PROCESOS_H__

#define MAX_CASILLAS 1029
#define MAX_MENSAJES 256
#include "mensajes.h"

typedef struct _ParametrosProcSocket {
    int connection_socket;
    mqd_t *workers;
	mqd_t casilla;
} ParametrosProcSocket;


typedef struct _ParametrosWorker {
    int id;
    mqd_t casilla;
    mqd_t *casillasWorkers;
} ParametrosWorker;

//aca tiene que ir un extern que referencie las casillas en procesos.c

mqd_t spawnear_procsocket(int, mqd_t*);
mqd_t spawnear_worker(int, mqd_t*);

#endif //__PROCESOS_H_
