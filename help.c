#include <dirent.h>
#include "headers/worker.h"
#include "headers/auxiliares.h"
#include <unistd.h>

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
    
    Msg respuesta = msgCreate(self, T_DEVUELVO_AYUDA, (void*)nombres, MAX_NOMBRE * MAX_ARCHIVOS);
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda LSD\n");
           
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

void helpDEL(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //data necesaria del worker
    int id = params.id;
    mqd_t self = params.casilla;
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    int res;
    if(esMio(id, rqst.nombre_archivo))
    {
        if(estaAbierto(data, rqst.nombre_archivo))
        {
            res = HELP_DEL_INUSE;
        }
        else
        {
            char cmd[MAX_NOMBRE + 15];
            sprintf(cmd, "rm data/worker%d/%s", id, rqst.nombre_archivo);
            system(cmd);
            res = HELP_DEL_OK;
        }
    }
    else
    {
        res = HELP_DEL_NOTFOUND;
    }
    
    Msg respuesta = msgCreate(self, T_DEVUELVO_AYUDA, &res, sizeof(int));
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda OPN\n");
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
    Abierto* abiertos = data->abiertos;
    int nAbiertos = data->nAbiertos;
    
    SList* sesiones = data->sesiones;
    int maxIDlocal = data->maxIDlocal;

    int FD;
    if(esMio(id, rqst.nombre_archivo))
    {
        //es mio
        if(estaAbierto(data, rqst.nombre_archivo))
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
            Abierto new = createAbierto(FD, rqst.nombre_archivo, self);
            
            //lo agrego a mi lista
            agregarAbiertoEnData(data, new);
        }
    }
    else
    {
        //no lo tengo
        FD = HELP_OPN_NOTFOUND;
    }
    
    Msg respuesta = msgCreate(self, T_DEVUELVO_AYUDA, &FD, sizeof(int));
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda DEL\n");
        
    //epilogo
    data->sesiones = sesiones;
    data->maxIDlocal = maxIDlocal;
}

void helpCLO(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    //handle...
    int res;
    // Reviso si es local
    if(esLocalFD(data, rqst.FD))
    {
        cerrarEnData(data, rqst.FD);
        res = HELP_CLO_OK;
    }
    else
    {
        res = HELP_CLO_NOTFOUND;
    }
    Msg respuesta = msgCreate(cumpa, T_DEVUELVO_AYUDA, &res, sizeof(int));
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda CLO\n");
}

void helpWRT(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    int res;
    // Reviso si es local
    if(esLocalFD(data, rqst.FD))
    {
        printf("quiero escribir el buffer -%s-\n", rqst.buffer);
        if(write(rqst.FD, rqst.buffer, rqst.cuanto_escribir))
            res = HELP_WRT_OK;
        else
            res = HELP_WRT_ERROR;
    }
    else
    {
        res = HELP_WRT_NOTFOUND;
    }
    Msg respuesta = msgCreate(cumpa, T_DEVUELVO_AYUDA, &res, sizeof(int));
    if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda WRT\n");
}

void helpREA(ParametrosWorker params, WorkerData *data, Msg *msg)
{
    //data necesaria del mensaje
    mqd_t cumpa = msg->remitente;
    Request rqst = *(Request*)(msg->datos);
    msgDestroy(msg);
    
    Leido leido;
    // Reviso si es local
    if(esLocalFD(data, rqst.FD))
    {
        if((leido.size = read(rqst.FD, leido.buffer, rqst.cuanto_leer)) <= 0)
        {
            leido.size = HELP_REA_ERROR;
        }
   }
   else
   {
       leido.size = HELP_REA_NOTFOUND;
   }

   Msg respuesta = msgCreate(cumpa, T_DEVUELVO_AYUDA, &leido, sizeof(Leido));
   if(msgSend(cumpa, respuesta) < 0)
        fprintf(stderr, "flashié send ayuda REA\n");
} 


