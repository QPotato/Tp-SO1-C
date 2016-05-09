//Estructuras y constantes que necesitan los workers.

#define N_WORKERS 6
#define MAX_NOMBRE 128
#define MAX_ARCHIVOS 1024

typedef struct
{
    int id;
    mqd_t casilla;
    SList* archivos;
} Sesion;

typedef struct
{
    SList* sesiones;
    int maxIDlocal = 0;
} WorkerData;
