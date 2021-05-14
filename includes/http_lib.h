/**************************************************************************
*	HTTP_LIB.H
*	Libreria de  RESOLUCION DE PETICIONES HTTP
*	Autores:
*			Adrian Rubio
**************************************************************************/ 

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>

#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404


/*Extensiones*/

/********
* FUNCIÓN: responde_servidor
* ARGS_IN: char * buf_res, int numero_respuesta,int minor_version,char * tipo_recurso,int tam, int flag,char* last_modified
* DESCRIPCIÓN: funcion para el envio rapido de respuestas de servidor HTTP
* ARGS_OUT: 
********/
void responde_servidor(char * buf_res, int numero_respuesta,int minor_version ,char * tipo_recurso, int tam, int flag,char*);
