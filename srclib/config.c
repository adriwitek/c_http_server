/**************************************************************************
*	CONFIG.C
*	Libreria de lectura de parametros de configuracion
*	Autores:
*			Adrian Rubio
**************************************************************************/
#include "../includes/config.h"
/********
* FUNCIÓN: config_server
* ARGS_IN: char * file_conf
* DESCRIPCIÓN: recoge los parametros de configuracion, en modo clave/valor del fichero pasado
* ARGS_OUT: config recogida
********/
Config config_server(char * file_conf) {
    static char *path = NULL;
    static long int clients = 1;
    static char *signature = NULL;
    static long int server_port = 8080;
    Config config;


    /* Estructura de configuración del servidor */
    cfg_opt_t opts[] = {
        CFG_SIMPLE_STR("server_root", &path),
        CFG_SIMPLE_INT("max_clients", &clients),
        CFG_SIMPLE_INT("listen_port", &server_port),
        CFG_SIMPLE_STR("server_signature", &signature),
        CFG_END()
    };
    cfg_t *cfg;


    /* Usamos los valores por defecto para la firma del servidor */
    signature = strdup("Servidor-Redes-2");
    
    cfg = cfg_init(opts, 0);

    /* Fichero de configuracion */
    cfg_parse(cfg, file_conf);

    //printf("server: %s\n", path);
    //printf("firma: %s\n", signature);
    //printf("max_clients: %ld\n", clients);
    //printf("port: %ld\n", server_port);

    /*Rellenamos la estructura auxiliar para el server*/
    config.server_root = strdup(path);
    config.server_port = server_port;
    config.server_signature = strdup(signature);
    config.clients = clients;

    cfg_free(cfg);
    free(signature);
    
    return config;
}
    
