#ifndef __MENSAJES_H__
#define __MENSAJES_H__
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "estructuras.h"

#define T_CASILLAS 1
#define T_AYUDA 2
#define T_DEVUELVO_AYUDA 3
#define T_REQUEST 0

/*
 Convencion de uso:
    1)el que envia, hace malloc para datos.
    2)el que recibe, cuando termina de usar el mensaje hace un msgDestroy
*/
typedef struct _Request Request;

typedef struct {
    int tipo;
    mqd_t remitente;
    void* datos;
} Msg;


//mandar un mensaje, devuelve -1 si falla
int msgSend(mqd_t queue, Msg mensaje);

//mandar un mensaje a todos los workers, version mala
int msgBroadcast(mqd_t remitente, mqd_t *receptores, Request* helpRequest);

//mandar un mensaje a todos los workers, version piola
int msgBroadcastPiola(mqd_t *receptores, Msg mensaje, size_t dataSize, void* arregloRespuestas, size_t size);

//recibir un mensaje, devuelve el size
int msgReceive(mqd_t queue, Msg *mensaje);

//todos los mensajes los tiene que destruir el que lo recibe
int msgDestroy(Msg* mensaje);
//crea una casilla
mqd_t createCasilla();

//destruye una casilla
int destroyCasilla(mqd_t cas);

Msg msgCreate(mqd_t remitente, int type, void *data, size_t size);

#endif //__MENSAJES_H__
