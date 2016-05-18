//Hay que revisar que de esto necesita el modulo y que no
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
//Nuestros
#include "headers/procesos.h"
#include "headers/worker.h"

#define MAX_CONEX 4096

//Un socket maneja una conexion.
//Ademas hay un socket de escucha para que no se mezcle con las conexiones.
mqd_t workers[N_WORKERS];
 
int dispatcher(int puerto)
{
    //mqd_t self = createCasilla();
    for(int i = 0; i < N_WORKERS; i++)
    {
        workers[i] = spawnear_worker(i, workers);
    }

    /*for(i = 0; i < N_WORKERS; i++)
    {
        for(j = 0; j < N_WORKERS; j++)
        {
            Msg mensaje;
            if(i != j)
            {
                mensaje.tipo = T_CASILLAS
                ;
                mensaje.remitente = self;
                mensaje.datos = malloc(sizeof(mqd_t));
                *((int*) mensaje.datos) = 1;
                if(msgSend(workers[i], mensaje) < 0)
                    fprintf(stderr, "flashié send workers\n");
            }
        }
    }*/
    
    /*int ack = 0;
    while(ack < N_WORKERS)
    {
        Msg mensaje;
        if(msgReceive(self, &mensaje) <= 0)
            fprintf(stderr, "flashié ack\n");
        
        //pointless chequeo
        if(mensaje.tipo == T_CASILLAS)
            ack++;
        msgDestroy(&mensaje);
    }*/
    
    
    //Preparacion extrania
    int listen_socket, connection_socket = -1;
    struct sockaddr_in servaddr;

    if ( (listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
        return -1;
    }
    int yes=1;
    //char yes='1';

    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(puerto);

    if ( bind(listen_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
        fprintf(stderr, "ECHOSERV: Error calling bind()\n");
        return -1;
    }

    if ( listen(listen_socket, 100) < 0 ) {
        fprintf(stderr, "ECHOSERV: Error calling listen()\n");
        return -1;
    }
    //Fin de preparacion extrania

    //Bucle que espera conexiones, crea un proc_socket y sigue esperando mas conexiones.
    for(;;) {
        if ( (connection_socket = accept(listen_socket, NULL, NULL) ) < 0 ) {
            fprintf(stderr, "ECHOSERV: Error calling accept()\n");
            return -1;
        }
        spawnear_procsocket(connection_socket, workers);
    }
    return 0;
}


int main(int argc, char *argv[])
{
    if(argc == 2)
        dispatcher(atoi(argv[1]));
    else
        printf("Bardeaste con la cantidad de argumentos!\nVa así:\n./tp [puerto]\n\n");
    return 0;
}
