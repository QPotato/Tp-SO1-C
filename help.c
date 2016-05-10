#include <dirent.h>
#include "headers/worker.h"

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
