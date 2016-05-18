#include <pthread.h>
#include <stdlib.h>
#include "headers/procesos.h"
void* worker(void*);
void* proc_socket(void*);
//aca tienen que estar definidas las casillas

int n_procesos = 0;
pthread_mutex_t mutex_n_procesos = PTHREAD_MUTEX_INITIALIZER;


//devuelve la id del proceso socket
mqd_t spawnear_procsocket(int connection_socket, mqd_t *workers) {
    pthread_mutex_lock(&mutex_n_procesos);
    ParametrosProcSocket *p = malloc(sizeof(ParametrosProcSocket));
    p->connection_socket = connection_socket;
    p->casilla = createCasilla();
    p->workers = workers;
    
    pthread_t t;
    pthread_create(&t, NULL, proc_socket, p);
    //aca le tiene que crear la casilla al proceso
    n_procesos++;
    pthread_mutex_unlock(&mutex_n_procesos);

    return p->casilla;
}

mqd_t spawnear_worker(int id, mqd_t* casillasWorkers) {
    pthread_mutex_lock(&mutex_n_procesos);
    ParametrosWorker *p = malloc(sizeof(ParametrosProcSocket));
    p->id = id;
    p->casilla = createCasilla();
    p->casillasWorkers = casillasWorkers;
    pthread_t t;
    pthread_create(&t, NULL, worker, p);
    //aca le tiene que crear la casilla al proceso
    n_procesos++;
    pthread_mutex_unlock(&mutex_n_procesos);

    return p->casilla;
}


