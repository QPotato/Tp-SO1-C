#include <dirent.h>
#include "headers/worker.h"
/*
    funciones necesarias
*/

//devuelve en char *nombres un string con todos los archivos separados por ' '
void getFiles(int id, mqd_t *workers, char *nombres)
{
    mqd_t self = workers[id];
    char dir[20];
    nombres[0] = '\0';
    sprintf(dir, "data/worker%d", id);
    DIR* d = opendir(dir);
    
    struct dirent *de;
    for(de = NULL; (de = readdir(d)) != NULL; )
    {
        if(!(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0))
        {
            strcat(nombres, de->d_name);
            strcat(nombres, " ");
        }
    }
    
    //creo el request
    Request helpRequest;
    helpRequest.con = LSD;
    msgBroadcast(self, workers, &helpRequest);
    
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        Msg helpReceive;
        if(msgReceive(self, &helpReceive) <= 0)
            fprintf(stderr, "flashié worker receive\n");
        if(helpReceive.tipo == T_DEVUELVO_AYUDA)
        {
            strcat(nombres, (char*)(helpReceive.datos));
            msgDestroy(&helpReceive);
        }
        else
        {
            if(msgSend(self, helpReceive) < 0)
                fprintf(stderr, "flashié devolviendome un mensaje (en LSD)\n");
        }
    }
    return;
}

int isConnected(mqd_t cumpa, SList* sesiones)
{
    return slist_contains(sesiones, (void*)&cumpa, mqd_t_comp);
}

/*
    handle CON
*/
void handleCON(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;
    
    //handle...
    if(isConnected(cumpa, sesiones))
    {
        //ya está conectado, devuelvo error.
        char res[128];
        sprintf(res, "Error: ya conectado.\n");
        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send respuesta cumpa CON\n");
    }
    else
    {
        //no está conectado, devuelvo la ID.
        int userID = maxIDlocal * N_WORKERS + id;
        maxIDlocal++;
        Sesion* sn = malloc(sizeof(Sesion));
        sn->casilla = cumpa;
        sn->abiertos = NULL;
        sesiones = slist_append(sesiones, (void*) sn);
        
        char res[128];
        sprintf(res, "OK ID %d\n", userID);
        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send respuesta cumpa CON\n");
    }
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
    return;
}


/*
    handle LSD
*/
void handleLSD(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    if(isConnected(cumpa, sesiones))
    {
        //está conectado, devuelvo los archivos.
        char nombres[MAX_NOMBRE * MAX_ARCHIVOS * N_WORKERS];
        getFiles(id, workers, nombres);
        strcat(nombres, "\n");
        Msg respuesta = msgCreate(self, T_REQUEST, nombres, strlen(nombres) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send respuesta cumpa LSD\n");
    }
    else
    {
        //no está conectado, devuelvo error.
        char res[128];
        sprintf(res, "Error: no conectado.\n");
        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send respuesta cumpa LSD\n");
    }    
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

