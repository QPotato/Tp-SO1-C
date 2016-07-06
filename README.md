# Tp-SO1-C
Version en C del TP de SOI

jueves 28:
-Implementamos LSD. Anda.
-Implementamos CRE. Anduvo magicamente y salto el error de nombres. Encontramos boludeces en el parser. Mallocs turbios.

10 mayo 2016:
    -pasamos el quilombo de worker.c a handle.c y help.c
    -armamos estructura WorkerData para comunicarnos con handle.c y help.c
    -intentamos hacer el OPN pero los archivos no tienen locks para open()
    -nueva estructura de archivos abiertos:
        un archivo abierto tiene
        ·int FD                             //el FD del archivo (que nos da unix)
        ·char nombre[MAX_NOMBRE];           //nombre del archivo
        ·mqd_t host;                        //el worker que tiene el archivo
    -el cliente guarda solo el FD
    -cada host guarda una lista de abiertos
    -si el archivo pedido lo tiene otro worker, yo lo guardo en mis abiertos pero me quedo la referencia a host.
    
3 de julio 2016:
    El buffer de lectura/escritura en la request lo pase de tamaño arbitrario a BUFF_SIZE. Esta piola el tamaño arbitrario, pero ya de por si los mensjaes que llegan del socket
    son de maximo BUFF_SIZE. Ademas, hace todo mas facil sin tener que compartir memoria entre procesos (antes pasabamos punteros a un proceso de un buffer en memoria de otro, puaj)
Bugs:
- segmentation fault raro con muchos OPN

TODO:
- buffer de mensajes que no eran respuesta
- getFiles con BroadcastPiola o al menos con sus correcciones
- helpOPN esta guardando el Abierto en el que ayuda. Eso va a hacer imposible pedir cierres masivos en BYE
- buffers tamaño fijo 4k para no compartir memoria
- Agregarle al parser el chequeo de que el SIZE no sea mayor a 1024
- De alguna manera dejar de usar BUFF_SIZE para el tamaño del buffer de lectura/escritura y para el tamaño maximo de mensaje que se puede recibir.

5 de julio 2016 (o por ahí):

    ANDA TODO LOCO.

Todavía hay que arreglar que comparte memoria en algún que otro lado.
Hay que testear un toque, pero parece que todo bien.
