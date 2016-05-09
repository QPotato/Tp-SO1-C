//Estructuras compartidas por el proc_socket, el parcer y los workers para comunicarse.
#ifndef __ESTRUCTURAS_H__
#define __ESTRUCTURAS_H__
#include <stdlib.h>
#include "worker.h"
typedef int Comand;

#define ERR -1
#define CON 0
#define LSD 1
#define DEL 2
#define CRE 3
#define OPN 4
#define WRT 5
#define REA 6
#define CLO 7
#define BYE 8

#define BUFF_SIZE 10000 //esto va en contra de eso de los tamaños arbitrarios de mensajes


typedef struct _Request {
    Comand con;
    int FD, cuanto_leer, cuanto_escribir;
    char *buffer; // <--------------- Esto lo tiene que liberar el qeu resuelva el WRT!!!!
    char nombre_archivo[MAX_NOMBRE];
} Request;
#endif //__ESTRUCTURAS_H__
