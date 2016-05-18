#include <dirent.h>
#include "headers/worker.h"
/*
    funciones necesarias
*/

// Funcion de comparacion. Devuelve 0 si y solo si la sesion a tiene como casilla b.
int mqd_t_comp(void* a, void* b)
{
    mqd_t x = ((Sesion*)a)->casilla;
    mqd_t y = *(mqd_t*)b;
    return y - x;
}

//devuelve en char *nombres un string con los archivos locales separados por ' '
void getLocalFiles(int id, mqd_t *workers, char *nombres)
{
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
    return;
}

//devuelve en char *nombres un string con todos los archivos separados por ' '
void getFiles(int id, mqd_t *workers, char *nombres)
{
    mqd_t self = workers[id];
    getLocalFiles(id, workers, nombres);
    
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

int buscarSesion(mqd_t cumpa, SList* sesiones)
{
    return slist_index(sesiones, (void*)&cumpa, mqd_t_comp);
}

/*
    handle CON
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
    int debug = buscarSesion(cumpa, sesiones);
    printf("index %d\n", debug);
    if(debug >= 0)
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
        sn->nAbiertos = 0;
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
            char res[128];
            sprintf(res, "OK\n");
            Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
            if(msgSend(cumpa, respuesta) < 0)
                fprintf(stderr, "flashié send respuesta cumpa CRE\n");
        }
        else
        {
            char res[128];
            sprintf(res, "ERROR el archivo ya existe\n");
            Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
            if(msgSend(cumpa, respuesta) < 0)
                fprintf(stderr, "flashié send respuesta cumpa CRE\n");
        }
    }
    else
    {
        char res[128];
        sprintf(res, "Error: no conectado.\n");
        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send request CRE\n");
    }
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}


/*
    handle DEL
*/
void handleDEL(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    //Request rqst = *(Request*)(msg->datos);
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
        char nombres[MAX_NOMBRE * MAX_ARCHIVOS * N_WORKERS];
        getFiles(id, workers, nombres);
    }
    else
    {
        char res[128];
        sprintf(res, "Error: no conectado.\n");
        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send request DEL\n");
    }
    
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
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

    //handle...
    int sesionID;
    if((sesionID = buscarSesion(cumpa, sesiones)) < 0)
    {
        char res[128];
        sprintf(res, "Error: no conectado.\n");
        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
        if(msgSend(cumpa, respuesta) < 0)
            fprintf(stderr, "flashié send request DEL\n");
    }
    else
    {
        char locales[MAX_NOMBRE * MAX_ARCHIVOS];
        getLocalFiles(id, workers, locales);
        if(strstr(locales, rqst.nombre_archivo) != NULL)            //TODO: fijarse que no esté abierto.
        {
            //tengo el archivo!
            char file[MAX_NOMBRE + 15];
            sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
            int FD = open(file, O_RDWR);
            
            //creo el abierto
            abierto new;
            new.fd = FD;
            strcpy(new.nombre, rqst.nombre_archivo);
            new.host = self;
            
            //lo agrego a mi lista
            data->abiertos[data->nAbiertos] = new;
            data->nAbiertos++;
            
            //modifico la sesion
            Sesion *ses = (Sesion *)SList_nth(sesiones, sesionID);
            ses->fd[ses->nAbiertos] = FD;
            ses->nAbiertos++;
            
            //envio la respuesta
            char res[128];
            sprintf(res, "OK FD %d\n", FD);
            Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
            if(msgSend(cumpa, respuesta) < 0)
                fprintf(stderr, "flashié send respuesta cumpa OPN\n");
        }
        else
        {
            //no lo tengo, quién lo tiene?
            printf("not implemented\n");
        }
    }
    
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
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado");
        
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
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado");
        
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
    int id = params.id;
    mqd_t self = params.casilla;
    mqd_t *workers;
    workers = params.casillasWorkers;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    //handle...
    printf("No implementado");
        
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}
