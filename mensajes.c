#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "headers/mensajes.h"
#include "headers/worker.h"
/*
Interfaz de mensajeria.
La convencion es que el que recibe un mensaje debe destruirlo o reenviarlo a alguien, porque msgCreate hace malloc.
*/

static unsigned _queue_id = 0;

//mandar un mensaje, devuelve -1 si falla
int msgSend(mqd_t queue, Msg mensaje)
{
    int prior = mensaje.tipo;
    char *mens = (char*)(&mensaje);
    if(mq_send(queue, mens, sizeof(Msg), prior) < 0)
        return -1;
    return 0;
}

//una funcion miembro masculino que rompe nuestras convenciones.
//crea mensajes para mandar a todo el mundo, asi los destroy multiples no tosquean.
//O sea era la idea, pero se deberia romper igual porque todos van a liberar helpRequest.
int msgBroadcast(mqd_t remitente, mqd_t *receptores, Request* helpRequest)
{
    for(int i = 0; i < N_WORKERS; i++)
    {
        if(receptores[i] != remitente)
        {
            if(msgSend(receptores[i], msgCreate(remitente, T_AYUDA, (void*) helpRequest, sizeof(Request))) < 0)
            {
                fprintf(stderr, "flashié pidiendo ayuda LSD\n");
                return -1;
            }    
        }
    }
    return 0;
}

void msgBroadcastPiola(mqd_t *receptores, Msg mensaje, size_t dataSize, void* arregloRespuestas, size_t size)
{
    mqd_t remitente = mensaje.remitente;
    for(int i = 0; i < N_WORKERS; i++)
    {
        if(receptores[i] != mensaje.remitente)
        {
            Msg mensajito = msgCreate(mensaje.remitente, mensaje.tipo, mensaje.datos, dataSize);
            msgSend(receptores[i], mensajito);
        }
    }
    msgDestroy(&mensaje);

    Msg buf[MAX_MSGBUF];
    int guardados = 0;

    //recibo toda la pavada.
    for(int i = 0; i < N_WORKERS - 1; i++)
    {
        Msg helpReceive;
        if(msgReceive(remitente, &helpReceive) <= 0)
            fprintf(stderr, "flashié worker receive\n");

        //mensajes que nos importan, los laburamos.
        if(helpReceive.tipo == T_DEVUELVO_AYUDA)
        {
            if(arregloRespuestas != NULL)
            {
                memcpy(arregloRespuestas, helpReceive.datos, size);
                arregloRespuestas += size;
                msgDestroy(&helpReceive);
            }
        }
        //mensajes que NO nos importan, los guardamos.
        else
        {
            buf[guardados++] = helpReceive;
            i--;
        }
    }

    //mando todos los mensajes que guarde.
    for(int i = 0; i < guardados; i++)
    {
        if(msgSend(remitente, buf[i]) < 0)
            fprintf(stderr, "flashié devolviendo los mensajes guardados en LSD\n");
    }
    return;
}

//recibir un mensaje, devuelve el size
int msgReceive(mqd_t queue, Msg *mensaje)
{
    int size;
    char *buf;
    struct mq_attr attr;
    mq_getattr(queue, &attr);
    buf = malloc(attr.mq_msgsize);
    size = mq_receive(queue, buf, attr.mq_msgsize, NULL);
    *mensaje = *((Msg*)buf);
    return size;
}

Msg msgCreate(mqd_t remit, int type, void *data, size_t size)
{
    Msg mensajito;
    mensajito.remitente = remit;
    mensajito.tipo = type;
    mensajito.datos = malloc(size);
    memcpy(mensajito.datos, data, size);
    return mensajito;
}

//Destruye el mensaje que ya se termino de usar.
int msgDestroy(Msg* mensaje){
    if(mensaje->datos != NULL) {    
        free(mensaje->datos);
        mensaje->datos = NULL;
        return 0;
    }
    else
        return -1;
}

mqd_t createCasilla()
{   
    mqd_t cas;
    char nombre[20];
    sprintf(nombre, "/q%u", _queue_id);
    _queue_id++;

    struct mq_attr newAttr;
    newAttr.mq_maxmsg = 5;
    newAttr.mq_msgsize = 2048;
    newAttr.mq_flags = 0;
    newAttr.mq_curmsgs = 0;
    if((cas = mq_open(nombre, O_RDWR | O_CREAT, 0666, &newAttr)) < 0)
        fprintf(stderr, "flashié mq_open en createCasilla con retorno %d\n", errno);

    mq_unlink(nombre);
    return cas;
}

int destroyCasilla(mqd_t cas)
{
    return mq_close(cas);
}

