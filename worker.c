#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "headers/mensajes.h"
#include "headers/procesos.h"
#include "headers/estructuras.h"
#include "headers/SList.h"
#include "headers/worker.h"
/*
    Como sabes si el primer mensaje son las casillas de los otros o ya es un mensaje de trabajo?
    Necesitamos guardar metadatos en los mensajes como los pattern matching de Erlang.

    Hacemos un poco mas larga msgDestroy para que tambien libere el void*
    Ponele que el mensaje que no es una casilla lo guardamos en una especie de buffer de mensajes para autoenviarnos cuando
    lleguen todas las casillas.
*/

int nada(){return 3;}

int mqd_t_comp(void* a, void* b)
{
    mqd_t x = ((Sesion*)a)->casilla;
    mqd_t y = *(mqd_t*)b;
    return y - x;
}
void* worker(void* params_v)
{
    // lo que hace worker/0
    // diccionario de sesiones
    mqd_t self;
    Msg msg;
    int id;
    ParametrosWorker params = *(ParametrosWorker*) params_v;
    id = params.id;
    self = params.casilla;
    system("mkdir data -p");
    
    char cmd[30];
    
    sprintf(cmd, "mkdir data/worker%d -p", id);
    system(cmd);
    
    int maxIDlocal = 0;
    SList* sesiones = NULL;
    
    WorkerData datos;
    datos.maxIDlocal = maxIDlocal;
    datos.sesiones = sesiones;
    
    while(1)
    {
        if(msgReceive(self, &msg) <= 0)
            fprintf(stderr, "flashié worker receive\n");
        
        if(msg.tipo == T_AYUDA)
        {
            Request rqst = *(Request*)(msg.datos);
            switch(rqst.con)
            {
                case LSD:
                    helpLSD(params, &datos, &msg);
                break;
                
                case DEL:
                break;
                
                case OPN:
                break;
                
                case WRT:
                break;
                
                case REA:
                break;
                
                case CLO:
                break;
            }
        }
        else if(msg.tipo == T_REQUEST)
        {               
            Request rqst = *(Request*)(msg.datos);
            switch(rqst.con)
            {
                case CON:
                    handleCON(params, &datos, &msg);
                break;
                
                case LSD:
                    handleLSD(params, &datos, &msg);
                break;
                
                case CRE:
                    handleCRE(params, &datos, &msg);
                break;
                
                case DEL:
                    handleDEL(params, &datos, &msg);
                break;
                
                case OPN:
                break;
                
                case WRT:
                break;
                
                case REA:
                break;
                
                case CLO:
                break;
                
                case BYE:
                break;
            }
        }
        else
            fprintf(stderr, "Acá llegó algo que no tenía que llegar\n");
    }
}

