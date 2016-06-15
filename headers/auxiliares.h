// Funcion de comparacion. Devuelve 0 si y solo si la sesion a tiene como casilla b.
int mqd_t_comp(void* a, void* b);

//devuelve en char *nombres un string con los archivos locales separados por ' '
void getLocalFiles(int id, mqd_t *workers, char *nombres);

//devuelve en char *nombres un string con todos los archivos separados por ' '
void getFiles(int id, mqd_t *workers, char *nombres);

//Devuelve 1 si el archivo existe, no importa quien lo tenga. Sino, 0.
int existeArchivo(int id, mqd_t *workers, char *nombre);

//Devuelve el indice de la sesion en la lista de sesiones o -1 si no esta.
int buscarSesion(mqd_t cumpa, SList* sesiones);

//Asumiendo que el archivo existe, retorna 1 si es local, 0 si lo tiene otro.
int esMio(int id, char *nombre);

//"Macro" para enviarle al proc_socket la respuesta a la request del usuario.
void enviarRespuesta(mqd_t remitente,mqd_t procSocket, char* resStr);

//Labura con las respuestas del broadcast que manda OPN
int handleOPNBroadcast(ParametrosWorker params, WorkerData *data, int* FDs, int sesionID);

//Labura con las respuestas del broadcast que manda DEL
int handleDELBroadcast(int* respuestas);

//Labura con las respuestas del broadcast que manda CLO
int handleCLOBroadcast(int* respuestas);

//Labura con las respuestas del broadcast que manda WRT
int handleWRTBroadcast(int* respuestas);

//Labura con las respuestas del broadcast que manda REA
int handleREABroadcast(char** respuestas, char* buffer);
