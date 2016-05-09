#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

//Headers nuestros
#include "headers/estructuras.h"
#include "headers/procesos.h"
#include "headers/worker.h"
Request parse(char*, int);

// La v es porque pthread fuerza a pasar punteros a void. Despues se traduce.
void* proc_socket(void *parametros_v)
{
    char mensaje_crudo[BUFF_SIZE];
    int res;
    Msg aWorker, deWorker;
    ParametrosProcSocket* parametros = (ParametrosProcSocket*) parametros_v;
    int connection_socket = parametros->connection_socket;
    mqd_t self = parametros->casilla, *workers = parametros->workers;
    
    srand(time(NULL));
    mqd_t elegido = workers[rand() % N_WORKERS];
    
    while(1)
    {
        // Despues de esto, buffer tiene el mensaje recibido, res tiene la cantidad de bytes leidos
        res = read(connection_socket, mensaje_crudo, BUFF_SIZE);
        if(res <= 0)
        {
            if(errno != 0)
                printf("E:read size 0, errno %d\n", errno);
            close(connection_socket);
            break;
        }
        mensaje_crudo[res]='\0';
		Request rqst = parse(mensaje_crudo, strlen(mensaje_crudo));

        if(rqst.con != -1)
        {
			//mandarle el request al worker
            aWorker.remitente = self;
            aWorker.tipo = T_REQUEST;
            aWorker.datos = malloc(sizeof(rqst));
            *(Request*)aWorker.datos = rqst;
            msgSend(elegido, aWorker);
			
            //recibo la respuesta del worker
            int msgSize;
            if((msgSize = msgReceive(self, &deWorker)) <= 0)
                printf("flashié msgSize\n");
                
            size_t sz = strlen((char*)deWorker.datos);
            write(connection_socket, deWorker.datos, sz);//habria que ver bien este size aca...
            if(msgDestroy(&deWorker) < 0)
                fprintf(stderr, "destroy deWorker");
		}
		else
		{
            write(connection_socket, "BAD REQUEST\n", 13);
		}

    }
    //recordar hacer un destroyCasilla despues del BYE
    return NULL;
}

