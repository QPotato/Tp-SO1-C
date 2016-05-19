#include <dirent.h>
#include "headers/worker.h"
#include "headers/auxiliares.h"

/*
    handle CON
    //TODO: revisar la cosa turbia con SesionID y UserID
*/
void handleCON(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;
    printf("soy %u", self);
    //handle...
    int SesionID = buscarSesion(cumpa, sesiones);
    printf("index %d\n", SesionID);
    if(SesionID >= 0)
    {
        //ya está conectado, devuelvo error.
        enviarRespuesta(self, cumpa, "Error: ya conectado.\n");
    }
    else
    {
        //no está conectado, devuelvo la ID.
        int userID = maxIDlocal * N_WORKERS + id;
        maxIDlocal++;
        Sesion* sn = malloc(sizeof(Sesion));
        sn->casilla = cumpa;
        sn->nAbiertos = 0;
        sesiones = slist_append(sesiones, (void*) sn);
        
        char res[128];
        sprintf(res, "OK ID %d\n", userID);
        enviarRespuesta(self, cumpa, res);
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
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    if(buscarSesion(cumpa, sesiones) >= 0)
    {
        //está conectado, devuelvo los archivos.
        char nombres[MAX_NOMBRE * MAX_ARCHIVOS * N_WORKERS];
        getFiles(id, workers, nombres);
        strcat(nombres, "\n");
        enviarRespuesta(self, cumpa, nombres);
    }
    else
    {
        //no está conectado, devuelvo error.
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
    }    
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}


/*
    handle CRE
*/
void handleCRE(ParametrosWorker params, WorkerData *data, Msg *msg)
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
    if(buscarSesion(cumpa, sesiones) >= 0)
    {
        char file[MAX_NOMBRE + 15];
        char nombres[(MAX_NOMBRE + 15) * MAX_ARCHIVOS * N_WORKERS];
        getFiles(id, workers, nombres);
        sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
        
        if(strstr(nombres, rqst.nombre_archivo) == NULL)
        {
            FILE* dummy = fopen(file, "a");
            fclose(dummy);
            enviarRespuesta(self, cumpa, "OK\n");
        }
        else
        {
            enviarRespuesta(self, cumpa, "ERROR el archivo ya existe\n");
        }
    }
    else
    {
        //no está conectado, devuelvo error.
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
    }
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}


/*
    handle DEL
    no implementado
*/
void handleDEL(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    //Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    //mqd_t *workers;
    //workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    if(buscarSesion(cumpa, sesiones) >= 0)
    {
        printf("No implementado DEL\n");
        enviarRespuesta(self, cumpa, "Error: no implementado.\n");
    }
    else
    {
        //no está conectado, devuelvo error.
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
    }
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

/*
    handle OPN
    a medio implementar
*/
void handleOPN(ParametrosWorker params, WorkerData *data, Msg *msg)
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
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    //handle...
    int sesionID;
    if((sesionID = buscarSesion(cumpa, sesiones)) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
    }
    else
    {
        char locales[MAX_NOMBRE * MAX_ARCHIVOS];
        getLocalFiles(id, workers, locales);
        if(strstr(locales, rqst.nombre_archivo) != NULL )            //TODO: fijarse que no esté abierto.
        {
            //tengo el archivo!
            if(estaAbierto(rqst.nombre_archivo, abiertos, nAbiertos))
            {
                enviarRespuesta(self, cumpa, "Error: archivo ya abierto\n");
            }
            else
            {
                char file[MAX_NOMBRE + 15];
                sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
                int FD = open(file, O_RDWR);
                
                //creo el abierto
                Abierto new;
                new.fd = FD;
                strcpy(new.nombre, rqst.nombre_archivo);
                new.host = self;
                
                //lo agrego a mi lista
                data->abiertos[data->nAbiertos] = new;
                data->nAbiertos++;
                
                //modifico la sesion
                Sesion *ses = (Sesion *)slist_nth(sesiones, sesionID);
                ses->fd[ses->nAbiertos] = FD;
                ses->nAbiertos++;
                
                //envio la respuesta
                char res[128];
                sprintf(res, "OK FD %d\n", FD);
                enviarRespuesta(self, cumpa, res);
            }
        }
        else
        {
            //no lo tengo, quién lo tiene?
            printf("not implemented, archivo existe pero lo tiene otro\n");
            enviarRespuesta(self, cumpa, "not implemented, archivo existe pero lo tiene otro\n");
        }
    }
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

/*
    handle REA
    No implementado
*/
void handleREA(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    //Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    //mqd_t *workers;
    //workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado\n");
    enviarRespuesta(self, cumpa, "Error: no implementado.\n");
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

/*
    handle WRT
    No implementado
*/
void handleWRT(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    //Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    //mqd_t *workers;
    //workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado\n");
    enviarRespuesta(self, cumpa, "Error: no implementado.\n");
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

/*
    handle CLO
    No implementado
*/
void handleCLO(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    //Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    //mqd_t *workers;
    //workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado\n");
    enviarRespuesta(self, cumpa, "Error: no implementado.\n");
        
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

/*
    handle BYE
    No implementado
*/
void handleBYE(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    //Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    //mqd_t *workers;
    //workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado\n");
    enviarRespuesta(self, cumpa, "Error: no implementado.\n");
        
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}
