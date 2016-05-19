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
