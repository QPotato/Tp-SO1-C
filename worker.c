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
    Ponele que el mensaje que no es una casilla lo guardamos en una especie de buffer de mensajes para autoenviarnos cuando
    lleguen todas las casillas.
*/

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
    datos.nAbiertos = 0;
    
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
                    helpDEL(params, &datos, &msg);
                break;
                
                case OPN:
                    helpOPN(params, &datos, &msg);
                break;
                
                case WRT:
                    helpWRT(params, &datos, &msg);
                break;
                
                case REA:
                    helpREA(params, &datos, &msg);
                break;
                
                case CLO:
                    helpCLO(params, &datos, &msg);
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
                    handleOPN(params, &datos, &msg);
                break;
                
                case WRT:
                    handleWRT(params, &datos, &msg);
                break;
                
                case REA:
                    handleREA(params, &datos, &msg);
                break;
                
                case CLO:
                    handleCLO(params, &datos, &msg);
                break;
                
                case BYE:
                    handleBYE(params, &datos, &msg);
                break;
            }
        }
        else
            fprintf(stderr, "Acá llegó algo que no tenía que llegar\n");
    }
}

