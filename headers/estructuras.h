//Estructuras compartidas por el proc_socket, el parcer y los workers para comunicarse.
#ifndef __ESTRUCTURAS_H__
#define __ESTRUCTURAS_H__
#include <stdlib.h>
#include "mensajes.h"
#include "SList.h"
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

#define BUFF_SIZE 1024 //esto va en contra de eso de los tamaños arbitrarios de mensajes              // si, y no me importa
#define MAX_NOMBRE 128
#define MAX_ARCHIVOS 1024
#define MAX_ABIERTOS MAX_ARCHIVOS


typedef struct _Request {
    Comand con;
    int FD, cuanto_leer, cuanto_escribir;
    char buffer[BUFF_SIZE];
    char nombre_archivo[MAX_NOMBRE];
} Request;

typedef struct _Leido {
    int size;
    char buffer[BUFF_SIZE];
} Leido;

typedef struct
{
    int fd;
    char nombre[MAX_NOMBRE];
    mqd_t host;
} Abierto;

Abierto createAbierto(int fd, const char* nombre, mqd_t host);

typedef struct
{
    mqd_t casilla;
    int fd[MAX_ABIERTOS];
    int nAbiertos;
} Sesion;

void agregarAbiertoEnSesion(Sesion* sesion, int nuevo);
int sesionTieneFD(Sesion* sesion, int FD);
void cerrarEnSesion(Sesion* sesion, int FD);

typedef struct
{
    SList* sesiones;
    int maxIDlocal;
    Abierto abiertos[MAX_ABIERTOS];
    int nAbiertos;
} WorkerData;

void agregarAbiertoEnData(WorkerData* data, Abierto nuevo);
int estaAbierto(WorkerData* data, const char* nombreAr);
int esLocalFD(WorkerData* data, int FD);
void cerrarEnData(WorkerData* data, int FD);

#endif //__ESTRUCTURAS_H__
