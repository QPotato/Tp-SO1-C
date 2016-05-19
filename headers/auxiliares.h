// Funcion de comparacion. Devuelve 0 si y solo si la sesion a tiene como casilla b.
int mqd_t_comp(void* a, void* b);

//devuelve en char *nombres un string con los archivos locales separados por ' '
void getLocalFiles(int id, mqd_t *workers, char *nombres);

//devuelve en char *nombres un string con todos los archivos separados por ' '
void getFiles(int id, mqd_t *workers, char *nombres);

//Devuelve el indice de la sesion en la lista de sesiones o -1 si no esta.
int buscarSesion(mqd_t cumpa, SList* sesiones);

//Devuelve 1 si hay un abierto de nombre nombreAr en abiertos.
int estaAbierto(const char* nombreAr, Abierto* abiertos, int nAbiertos);

//"Macro" para enviarle al proc_socket la respuesta a la request del usuario.
void enviarRespuesta(mqd_t remitente,mqd_t procSocket, char* resStr);
