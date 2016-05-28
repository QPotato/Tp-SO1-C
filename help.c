#include <dirent.h>
#include "headers/worker.h"
#include "headers/auxiliares.h"

void helpLSD(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    char dir[20];
    char nombres[MAX_NOMBRE * MAX_ARCHIVOS];
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
    
    Msg respuesta = msgCreate(self, T_DEVUELVO_AYUDA, (void*)nombres, strlen(nombres) + 1);
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda LSD\n");
           
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

void helpOPN(ParametrosWorker params, WorkerData *data, Msg *msg)
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
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    char nombres[MAX_ARCHIVOS * MAX_NOMBRE];
    getLocalFiles(id, workers, nombres);
    int FD;
    if(strstr(nombres, rqst.nombre_archivo) != NULL )
    {
        //es mio
        if(estaAbierto(rqst.nombre_archivo, abiertos, nAbiertos))
        {
            FD = HELP_OPN_INUSE; //lo tengo pero ya esta abierto
        }
        else
        {
            //abro el archivo
            char file[MAX_NOMBRE + 15];
            sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
            FD = open(file, O_RDWR);
            
            //creo el abierto
            Abierto new;
            new.fd = FD;
            strcpy(new.nombre, rqst.nombre_archivo);
            new.host = self;
            
            //lo agrego a mi lista
            data->abiertos[data->nAbiertos] = new;
            data->nAbiertos++;
        }
    }
    else
    {
        FD = HELP_OPN_NOTFOUND; //no lo tengo
    }
    
    Msg respuesta = msgCreate(cumpa, T_DEVUELVO_AYUDA, &FD, sizeof(int));
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda OPN\n");
        
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

void helpCLO(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    /*
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;
        
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
    */
    printf("Llamada a ayuda no implementada");
}

void helpDEL(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    printf("Llamada a ayuda no implementada");
}

void helpREA(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    printf("Llamada a ayuda no implementada");
}


void helpWRT(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    printf("Llamada a ayuda no implementada");
}

