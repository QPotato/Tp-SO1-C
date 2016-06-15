#include <dirent.h>
#include "headers/worker.h"
#include "headers/auxiliares.h"
#include <unistd.h>

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
        return;
    }

    //Todo ok, lo conecto y devuelvo la ID.
    int userID = maxIDlocal * N_WORKERS + id;
    data->maxIDlocal++;
    
    Sesion* sn = malloc(sizeof(Sesion));
    sn->casilla = cumpa;
    sn->nAbiertos = 0;
    data->sesiones = slist_append(sesiones, (void*) sn);
    
    char res[128];
    sprintf(res, "OK ID %d\n", userID);
    enviarRespuesta(self, cumpa, res);
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
    
    // Me fijo que este conectado
    if(buscarSesion(cumpa, sesiones) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        return;
    }
    
    // Todo ok, devuelvo los archivos.
    char nombres[MAX_NOMBRE * MAX_ARCHIVOS * N_WORKERS];
    getFiles(id, workers, nombres);
    strcat(nombres, "\n");
    enviarRespuesta(self, cumpa, nombres);
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
    
    // Me fijo que este conectado
    if(buscarSesion(cumpa, sesiones) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        return;
    }
    
    // Reviso que no exista ya ese archivo
    if(existeArchivo(id, workers, rqst.nombre_archivo))
    {
        enviarRespuesta(self, cumpa, "Error: el archivo ya existe\n");
        return;
    }
    
    // Todo ok, lo creo.
    char file[MAX_NOMBRE + 15];
    sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
    FILE* dummy = fopen(file, "a");
    fclose(dummy);
    enviarRespuesta(self, cumpa, "OK\n");
}


/*
    handle DEL
    no implementado
*/
void handleDEL(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    
    // Me fijo que este conectado
    if(buscarSesion(cumpa, sesiones) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        return;
    }
    
    // Reviso que exista ese archivo
    if(!existeArchivo(id, workers, rqst.nombre_archivo))
    {
        enviarRespuesta(self, cumpa, "Error: archivo inexistente\n");
        return;
    }
    // Todo ok, lo tengo que borrar.
    if(esMio(id, rqst.nombre_archivo))
    {
        if(estaAbierto(data, rqst.nombre_archivo))
        {
            enviarRespuesta(self, cumpa,"Error: Archivo abierto\n");
            return;
        }
        char cmd[MAX_NOMBRE + 15];
        sprintf(cmd, "rm data/worker%d/%s", id, rqst.nombre_archivo);
        system(cmd);
        enviarRespuesta(self, cumpa,"OK\n");
        return;
    }
    else
    {
        int respuestas[N_WORKERS - 1];
        Msg helpMsg = msgCreate(self, T_AYUDA, &rqst, sizeof(rqst));
        msgBroadcastPiola(workers, helpMsg, sizeof(rqst), respuestas, sizeof(int));
        int res = handleDELBroadcast(respuestas);
        switch(res)
        {
            case HELP_DEL_OK:
                enviarRespuesta(self, cumpa,"OK\n");
                break;
            case HELP_DEL_INUSE:
                enviarRespuesta(self, cumpa,"Error: Archivo abierto\n");
                break;
            case HELP_DEL_NOTFOUND:
                enviarRespuesta(self, cumpa,"Error: Se pudrio todo\n");
                printf("Flashiamos en CLO\n");
                break;
        }
        return;
    }
}

/*
    handle OPN
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
    // Reviso que este conectado y de paso averiguo su ID.
    if((sesionID = buscarSesion(cumpa, sesiones)) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        return;
    }
    
    // Reviso que exista ese archivo
    if(!existeArchivo(id, workers, rqst.nombre_archivo))
    {
        enviarRespuesta(self, cumpa, "Error: archivo inexistente\n");
        return;
    }
    // Lo tengo yo?
    if(esMio(id, rqst.nombre_archivo))
    {
        // Reviso que no este ya abierto.
        if(estaAbierto(data, rqst.nombre_archivo))
        {
            enviarRespuesta(self, cumpa, "Error: archivo ya abierto\n");
            return;
        }
        
        //lo abro en el FS local
        char file[MAX_NOMBRE + 15];
        sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
        int FD = open(file, O_RDWR);
        
        //creo el abierto
        Abierto new = createAbierto(FD, rqst.nombre_archivo, self);
        
        //lo agrego a mi lista
        agregarAbiertoEnData(data, new);
        
        //modifico la sesion
        Sesion *ses = (Sesion *)slist_nth(sesiones, sesionID);
        agregarAbiertoEnSesion(ses, FD);
        
        //envio la respuesta
        char res[128];
        sprintf(res, "OK FD %d\n", FD);
        enviarRespuesta(self, cumpa, res);
        return;
    }
    else
    {
        // No lo tengo, quién lo tiene?
        int FDs[N_WORKERS - 1];
        Msg helpMsg = msgCreate(self, T_AYUDA, &rqst, sizeof(rqst));
        msgBroadcastPiola(workers, helpMsg, sizeof(rqst), FDs, sizeof(int));
        int FD = handleOPNBroadcast(params, data, FDs, sesionID);
        char respuesta[20];
        
        switch(FD)
        {
            case HELP_OPN_NOTFOUND:
                enviarRespuesta(self, cumpa,"Error: Se pudrio todo\n");
                printf("Flashiamos en OPN\n");
                break;
                
            case HELP_OPN_INUSE:
                enviarRespuesta(self, cumpa,"Error: archivo ya abierto\n");
                break;
                
            default:
                sprintf(respuesta, "OK FD %d\n", FD);
                enviarRespuesta(self, cumpa, respuesta);
        }
    }
}

/*
    handle CLO
*/
void handleCLO(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;

    //handle...
    
    // Reviso que este conectado y de paso averiguo su ID.
    int sesionID;
    if((sesionID = buscarSesion(cumpa, sesiones)) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        return;
    }
    Sesion *ses = (Sesion *)slist_nth(sesiones, sesionID);
    
    // Reviso que la sesion tenga un abierto con ese FD
    if(!sesionTieneFD(ses, rqst.FD))
    {
        enviarRespuesta(self, cumpa, "Error: FD no corresponde a un archivo abierto.\n");
        return;
    }
    cerrarEnSesion(ses, rqst.FD);
    
    // Me fijo si el archivo es local
    if(esLocalFD(data, rqst.FD))
    {
        cerrarEnData(data, rqst.FD);
        enviarRespuesta(self, cumpa, "OK\n");
    }
    else
    {
        int respuestas[N_WORKERS - 1];
        Msg helpMsg = msgCreate(self, T_AYUDA, &rqst, sizeof(rqst));
        msgBroadcastPiola(workers, helpMsg, sizeof(rqst), respuestas, sizeof(int));
        int res = handleCLOBroadcast(respuestas);
        switch(res)
        {
            case HELP_CLO_OK:
                enviarRespuesta(self, cumpa,"OK\n");
                break;
            case HELP_DEL_NOTFOUND:
                enviarRespuesta(self, cumpa,"Error: Se pudrio todo\n");
                printf("Flashiamos en CLO\n");
                break;
        }
    }
}

/*
    handle WRT
*/
void handleWRT(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;

    //handle...
    // Reviso que este conectado y de paso averiguo su ID.
    int sesionID;
    if((sesionID = buscarSesion(cumpa, sesiones)) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        free(rqst.buffer);
        return;
    }
    Sesion *ses = (Sesion *)slist_nth(sesiones, sesionID);
    
    // Reviso que la sesion tenga un abierto con ese FD
    if(!sesionTieneFD(ses, rqst.FD))
    {
        enviarRespuesta(self, cumpa, "Error: FD no corresponde a un archivo abierto.\n");
        free(rqst.buffer);
        return;
    }
    
    // Me fijo si el archivo es local
    if(esLocalFD(data, rqst.FD))
    {
        if(write(rqst.FD, rqst.buffer, rqst.cuanto_escribir))
            enviarRespuesta(self, cumpa, "OK\n");
        else
            enviarRespuesta(self, cumpa, "Error de escritura\n");
    }
    else
    {
        int respuestas[N_WORKERS - 1];
        Msg helpMsg = msgCreate(self, T_AYUDA, &rqst, sizeof(rqst));
        msgBroadcastPiola(workers, helpMsg, sizeof(rqst), respuestas, sizeof(int));
        int res = handleWRTBroadcast(respuestas);
        switch(res)
        {
            case HELP_WRT_OK:
                enviarRespuesta(self, cumpa,"OK\n");
                break;
            case HELP_WRT_ERROR:
                enviarRespuesta(self, cumpa, "Error de escritura\n");
                printf("Flashiamos en WRT\n");
                break;
        }
    }
    free(rqst.buffer);
}

/*
    handle REA
    No implementado
*/
void handleREA(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    /*
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    //int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;

    //handle...
    // Reviso que este conectado y de paso averiguo su ID.
    int sesionID;
    if((sesionID = buscarSesion(cumpa, sesiones)) < 0)
    {
        enviarRespuesta(self, cumpa, "Error: no conectado.\n");
        free(rqst.buffer);
        return;
    }
    Sesion *ses = (Sesion *)slist_nth(sesiones, sesionID);
    
    // Reviso que la sesion tenga un abierto con ese FD
    if(!sesionTieneFD(ses, rqst.FD))
    {
        enviarRespuesta(self, cumpa, "Error: FD no corresponde a un archivo abierto.\n");
        free(rqst.buffer);
        return;
    }
    
    // Me fijo si el archivo es local
    if(esLocalFD(data, rqst.FD))
    {
        char* buffer = malloc((rqst.cuanto_leer + 1) * sizeof(char));
        int rdSize;
        if((rdSize = read(rqst.FD, buffer, rqst.cuanto_leer)) >= 0)
        {
            buffer[rdSize] = '\0';
            char res = malloc((rdSize + 20) * sizeof(char));
            sprintf(res, "OK FD %s\n", buffer);
            enviarRespuesta(self, cumpa, res);
            free(res);
        }
        else
            enviarRespuesta(self, cumpa, "Error de lectura\n");
        free(buffer);
    }
    else
    {
        char* respuestas[N_WORKERS - 1];
        char* buffer = malloc((rqst.cuanto_leer + 1) * sizeof(char));
        int rdSize;
        Msg helpMsg = msgCreate(self, T_AYUDA, &rqst, sizeof(rqst));
        msgBroadcastPiola(workers, helpMsg, sizeof(rqst), respuestas, sizeof(char*));
        int resultado = handleREABroadcast(respuestas, buffer);
        switch(resltado)
        {
            case HELP_REA_OK:
                buffer[rdSize] = '\0';
                char res = malloc((strlen(buffer) + 20) * sizeof(char));
                sprintf(res, "OK FD %s\n", buffer);
                enviarRespuesta(self, cumpa, res);
                free(res);
                break;
            case HELP_REA_ERROR:
                enviarRespuesta(self, cumpa, "Error de lectura\n");
                printf("Flashiamos en REA\n");
                break;
        }
        free(buffer);
    }
    */
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
