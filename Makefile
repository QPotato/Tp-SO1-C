CC=gcc
LOS_H= headers/estructuras.h headers/workers.h headers/procesos.h headers/mensajes.h headers/SList.h headers/auxiliares.h
LOS_O= handle.o dispatcher.o parser.o procesos.o proc_socket.o worker.o mensajes.o SList.o help.o auxiliares.o estructuras.o
CFLAGS= -std=c99 -g -Wall -Wno-unused-value -pthread -lrt

tp: $(LOS_O)
	gcc $(LOS_O) $(CFLAGS) -o tp 

%.o: %.c $(LOS_H)
	$(CC) $(CFLAGS) -c -o $@ $< 
	
clean:
	rm tp $(LOS_O)
    
