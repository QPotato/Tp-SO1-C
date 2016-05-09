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
    mqd_t y = ((Sesion*)a)->casilla;
    return y - x;
}

int getFiles(int id, mqd_t *workers, char *nombres)
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
    return 0;
}

void* worker(void* params_v)
{
    // lo que hace worker/0
    // diccionario de sesiones
    mqd_t self;
    Msg msg;
    int id;
    mqd_t *workers;
    ParametrosWorker params = *(ParametrosWorker*) params_v;
    id = params.id;
    self = params.casilla;
    workers = params.casillasWorkers;
    //recibirCasillas(casillasWorkers, self);
    
    system("mkdir data -p");
    
    char cmd[30];
    
    sprintf(cmd, "mkdir data/worker%d -p", id);
    system(cmd);
    
    int maxIDlocal = 0;
    
    SList* sesiones = NULL;
    
    while(1)
    {
        if(msgReceive(self, &msg) <= 0)
            fprintf(stderr, "flashié worker receive\n");
        
        if(msg.tipo == T_AYUDA)
        {
            mqd_t cumpa = msg.remitente;
            Request rqst = *(Request*)(msg.datos);
            msgDestroy(&msg);
            
            switch(rqst.con)
            {
                case LSD:
                    nada();
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
                    
                    msg = msgCreate(self, T_DEVUELVO_AYUDA, (void*)nombres, strlen(nombres) + 1);
                    if(msgSend(cumpa, msg) < 0)
                        fprintf(stderr, "flashié send ayuda LSD\n");
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
            /*mqd_t cumpa = msg.remitente;
            msg.remitente = self;
            if(msgSend(cumpa, msg) < 0)
                fprintf(stderr, "flashié ECHO\n");*/
            
            mqd_t cumpa = msg.remitente;
            Request rqst = *(Request*)(msg.datos);
            msgDestroy(&msg);
            
            
            switch(rqst.con)
            {
                case CON:
                    nada();
                    if(slist_contains(sesiones, (void*)&cumpa, mqd_t_comp))
                    {
                        char res[128];
                        sprintf(res, "Error: ya conectado.\n");
                        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
                        if(msgSend(cumpa, respuesta) < 0)
                            fprintf(stderr, "flashié send respuesta cumpa CON\n");
                    }
                    else
                    {
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
                break;
                
                case LSD:
                    if(slist_contains(sesiones, (void*)&cumpa, mqd_t_comp))
                    {
                        nada();
                        char nombres[MAX_NOMBRE * MAX_ARCHIVOS * N_WORKERS];
                        
                        getFiles(id, workers, nombres);
                        
                        strcat(nombres, "\n");
                        Msg respuesta = msgCreate(self, T_REQUEST, nombres, strlen(nombres) + 1);
                        if(msgSend(cumpa, respuesta) < 0)
                            fprintf(stderr, "flashié send respuesta cumpa LSD\n");
                    }
                    else
                    {
                        char res[128];
                        sprintf(res, "Error: no conectado.\n");
                        Msg respuesta = msgCreate(self, T_REQUEST, res, strlen(res) + 1);
                        if(msgSend(cumpa, respuesta) < 0)
                            fprintf(stderr, "flashié send respuesta cumpa LSD\n");
                    }
                break;
                
                case DEL:
                    if(slist_contains(sesiones, (void*)&cumpa, mqd_t_comp))
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
                break;
                
                case CRE:
                    if(slist_contains(sesiones, (void*)&cumpa, mqd_t_comp))
                    {
                        char file[MAX_NOMBRE + 15];
                        char nombres[(MAX_NOMBRE + 15) * MAX_ARCHIVOS * N_WORKERS];
                        getFiles(id, workers, nombres);
                        sprintf(file, "data/worker%d/%s", id, rqst.nombre_archivo);
                        
                        printf("rqst.nombre_archivo -%s-\n", rqst.nombre_archivo);
                        
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

