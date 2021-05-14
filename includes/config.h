/**************************************************************************
*	CONFIG.H
*	Libreria de lectura de parametros de configuracion
*	Autores:
*			Adrian Rubio
**************************************************************************/
#ifndef CONFIG_H

#define CONFIG_H

#define _GNU_SOURCE

#include <confuse.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

/*Estructura auxiliar para la funcion del modulo*/
typedef struct Config {
    char * server_root;
    int clients;
    char * server_signature;
    int server_port;
}Config;

/********
* FUNCIÓN: config_server
* ARGS_IN: char * file_conf
* DESCRIPCIÓN: recoge los parametros de configuracion, en modo clave/valor del fichero pasado
* ARGS_OUT: config recogida
********/
Config config_server(char * file_conf);




#endif