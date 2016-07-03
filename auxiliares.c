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
void getLocalFiles(int id, char *nombres)
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
    getLocalFiles(id, nombres);
    
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

//Devuelve 1 si el archivo existe, no importa quien lo tenga. Sino, 0.
int existeArchivo(int id, mqd_t *workers, char *nombre)
{
    char nombres[(MAX_NOMBRE + 15) * MAX_ARCHIVOS * N_WORKERS];
    getFiles(id, workers, nombres);
    
    if(strstr(nombres, nombre) == NULL)
        return 0;
    else
        return 1;
}

//Devuelve el indice de la sesion en la lista de sesiones o -1 si no esta.
int buscarSesion(mqd_t cumpa, SList* sesiones)
{
    return slist_index(sesiones, (void*)&cumpa, mqd_t_comp);
}

//Asumiendo que el archivo existe, retorna 1 si es local, 0 si lo tiene otro.
int esMio(int id, char *nombre)
{
    char locales[MAX_NOMBRE * MAX_ARCHIVOS];
    getLocalFiles(id, locales);
    //Si tengo el archivo...
    if(strstr(locales, nombre) != NULL )
        return 1;
    else
        return 0;
}

//"Macro" para enviarle al proc_socket la respuesta a la request del usuario.
void enviarRespuesta(mqd_t remitente,mqd_t procSocket, char* resStr)
{
    Msg respuesta = msgCreate(remitente, T_REQUEST, resStr, strlen(resStr) + 1);
    if(msgSend(procSocket, respuesta) < 0)
        fprintf(stderr, "flashié enviarRespuesta\n");
}

int handleDELBroadcast(int* respuestas)
{
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        if(respuestas[i] == HELP_DEL_INUSE)
        {
            return HELP_DEL_INUSE;
        }
        if(respuestas[i] == HELP_DEL_OK)
        {
            return HELP_DEL_OK;
        }
    }
    return HELP_DEL_NOTFOUND;
}

int handleOPNBroadcast(ParametrosWorker params, WorkerData *data, int* FDs, int sesionID)
{
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        if(FDs[i] == HELP_OPN_INUSE)
        {
            return HELP_OPN_INUSE;
        }
        if(FDs[i] >= 0)
        {
            SList* sesiones = data->sesiones;
            Sesion *ses = (Sesion *)slist_nth(sesiones, sesionID);
            ses->fd[ses->nAbiertos] = FDs[i];
            ses->nAbiertos++;
            return FDs[i];
        }
    }
    return HELP_OPN_NOTFOUND;
}

int handleCLOBroadcast(int* respuestas)
{
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        if(respuestas[i] == HELP_CLO_OK)
        {
            return HELP_CLO_OK;
        }
    }
    return HELP_CLO_NOTFOUND;
}

int handleWRTBroadcast(int* respuestas)
{
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        if(respuestas[i] == HELP_WRT_OK)
        {
            return HELP_WRT_OK;
        }
        if(respuestas[i] == HELP_WRT_ERROR)
        {
            return HELP_WRT_ERROR;
        }
    }
    return HELP_WRT_NOTFOUND;
}

/*
    Maneja las respuestas al broadcast que hace el handler de REA cuando el FD es valido pero no local.
    Todos los workers contestan con un Leido.
        - Si no tienen el archivo, size = HELP_REA_NOTFOUND
        - Si lo tienen pero hay error de lectura, size = HELP_REA_ERROR
*/
int handleREABroadcast(Leido *respuestas, char* buffer, int* rdSize)
{    
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        if(respuestas[i].size >= 0)
        {
            // Este tenia el archivo y pudo leer! Nos copiamos el resultado.
            memcpy(buffer, respuestas[i].buffer, respuestas[i].size);
            *rdSize = respuestas[i].size;
            return HELP_REA_OK;
        }
    }
    return HELP_REA_ERROR;
}
