/*
PARSER
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "headers/estructuras.h"

//devuelve el índice donde termina la primer palabra de una cadena
int firstWordIndex(char *cadena)
{
    int size = strlen(cadena);
    int pos = 0;
    if(cadena[0] == ' ')
        return 0;
    while(cadena[pos] != ' ' && pos < size)
        pos++;
    return pos;
}

//devuelve la primer palabra de una cadena
void firstWord(char *cadena, char *word)
{
    int pos;
    pos = firstWordIndex(cadena);
    memcpy((void*)word, (void*)cadena, pos);
    word[pos] = '\0';
    return;
}

int isNumber(char *cadena) {
	int i;
	for(i = 0; i < strlen(cadena); i++) {
		if(isdigit(cadena[i]) == 0)
			return 0;
	}
	return 1;
}

//devuelve el Comand al que refiere la primer palabra del mensaje (-1 si no es un comando válido)
Comand getComand(char *cadena) {
    if(strcmp(cadena, "CON") == 0)
        return CON;
    if(strcmp(cadena, "LSD") == 0)
        return LSD;
	if(strcmp(cadena, "DEL") == 0)
        return DEL;
	if(strcmp(cadena, "CRE") == 0)
        return CRE;
    if(strcmp(cadena, "OPN") == 0)
        return OPN;
	if(strcmp(cadena, "WRT") == 0)
        return WRT;
    if(strcmp(cadena, "REA") == 0)
        return REA;
    if(strcmp(cadena, "CLO") == 0)
        return CLO;
    if(strcmp(cadena, "BYE") == 0)
        return BYE;
    return ERR;
}

//devuelve la cadena que contiene los argumentos (cadena vacía si no hay argumentos)
void getArgs(char *cadena, char *args) {
    int size = strlen(cadena);
    int pos = firstWordIndex(cadena) + 1;
    if(pos == size + 1)
        strcpy(args, "");
    else
        strcpy(args, &cadena[pos]);
}

//devuelve el Request que representa un mensaje. Si el mensaje no representa un Request válido, devuelve un Request con comando -1
Request parse(char *cadena) {
    Request rq;
    int size = strlen(cadena);
    
    //filtrar pavadas
    int i;
    for(i = 0; i < size; i++)
    {
        if(cadena[i] < ' ' || cadena[i] > '~')
        {
            cadena[i] = '\0';
            size = i;
            break;
        }
    }
    
    //obtengo el comando
    char scmd[256];
    firstWord(cadena, scmd);
    Comand cmd = getComand(scmd);
    
    //obtengo los argumentos
    char args[5][BUFF_SIZE];
    char sobra[BUFF_SIZE];
    getArgs(cadena, sobra);
    firstWord(sobra, args[0]);
    getArgs(sobra, sobra);
    firstWord(sobra, args[1]);
    getArgs(sobra, sobra);
    firstWord(sobra, args[2]);
    getArgs(sobra, sobra);
    firstWord(sobra, args[3]);
    getArgs(sobra, args[4]);
    
	//inicializo el request
    rq.con = cmd;
    rq.FD = -1;
    rq.cuanto_leer = -1;
    rq.cuanto_escribir = -1;
    rq.buffer = NULL;
    
    //chequeo que el request sea correcto y lleno los campos necesarios de la estructura
    switch(cmd) {
        case ERR:
        break;
        
        case CON:
        if( strcmp(args[0], "") != 0 ||
            strcmp(args[1], "") != 0 ||
            strcmp(args[2], "") != 0 ||
            strcmp(args[3], "") != 0 ||
            strcmp(args[4], "") != 0 ) {
            rq.con = ERR;
        }
        break;
        
        case LSD:
        if( strcmp(args[0], "") != 0 ||
            strcmp(args[1], "") != 0 ||
            strcmp(args[2], "") != 0 ||
            strcmp(args[3], "") != 0 ||
            strcmp(args[4], "") != 0 ) {
            rq.con = ERR;
        }
        break;
        
        case DEL:
        if( strcmp(args[1], "") != 0 ||
            strcmp(args[2], "") != 0 ||
            strcmp(args[3], "") != 0 ||
            strcmp(args[4], "") != 0 ) {
            rq.con = ERR;
        }
        strcpy(rq.nombre_archivo, args[0]);
        break;
        
        case CRE:
        if( strcmp(args[1], "") != 0 ||
            strcmp(args[2], "") != 0 ||
            strcmp(args[3], "") != 0 ||
            strcmp(args[4], "") != 0 ) {
            rq.con = ERR;
        }
        strcpy(rq.nombre_archivo, args[0]);
        break;
        
        case OPN:
        if( strcmp(args[1], "") != 0 ||
            strcmp(args[2], "") != 0 ||
            strcmp(args[3], "") != 0 ||
            strcmp(args[4], "") != 0 ) {
            rq.con = ERR;
        }
        strcpy(rq.nombre_archivo, args[0]);
        break;
        
        case WRT:
        if( strcmp(args[0], "FD") != 0   ||
            strcmp(args[2], "SIZE") != 0 ||
			isNumber(args[1]) == 0       ||
			isNumber(args[3]) == 0       ) {
            rq.con = ERR;
        }
        rq.FD = atoi(args[1]);
        rq.cuanto_escribir = atoi(args[3]);
        rq.buffer = malloc(strlen(args[4] + 1));
        strcpy(rq.nombre_archivo, args[0]);
        break;
        
        case REA:
        if( strcmp(args[0], "FD") != 0   ||
            strcmp(args[2], "SIZE") != 0 ||
            strcmp(args[4], "") != 0     ||
			isNumber(args[1]) == 0       ||
			isNumber(args[3]) == 0       ) {
            rq.con = ERR;
        }
        rq.FD = atoi(args[1]);
        rq.cuanto_leer = atoi(args[3]);
        break;
        
        case CLO:
        if( strcmp(args[0], "FD") != 0 ||
            strcmp(args[2], "") != 0   ||
            strcmp(args[3], "") != 0   ||
            strcmp(args[4], "") != 0   ||
			isNumber(args[1]) == 0     ) {
            rq.con = ERR;
        }
        rq.FD = atoi(args[1]);
        break;
		
        case BYE:
        if( strcmp(args[0], "") != 0 ||
            strcmp(args[1], "") != 0 ||
            strcmp(args[2], "") != 0 ||
            strcmp(args[3], "") != 0 ||
            strcmp(args[4], "") != 0 ) {
            rq.con = ERR;
        }
        break;
    }
    return rq;
}

