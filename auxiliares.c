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

//Devuelve el indice de la sesion en la lista de sesiones o -1 si no esta.
int buscarSesion(mqd_t cumpa, SList* sesiones)
{
    return slist_index(sesiones, (void*)&cumpa, mqd_t_comp);
}

//Devuelve 1 si hay un abierto de nombre nombreAr en abiertos.
int estaAbierto(const char* nombreAr, Abierto* abiertos, int nAbiertos)
{
    for(int i = 0; i < nAbiertos; i++)
        if(strcmp(nombreAr, abiertos[i].nombre) == 0)
            return 1;
    return 0;
}

//"Macro" para enviarle al proc_socket la respuesta a la request del usuario.
void enviarRespuesta(mqd_t remitente,mqd_t procSocket, char* resStr)
{
    Msg respuesta = msgCreate(remitente, T_REQUEST, resStr, strlen(resStr) + 1);
    if(msgSend(procSocket, respuesta) < 0)
        fprintf(stderr, "flashié enviarRespuesta\n");
}
