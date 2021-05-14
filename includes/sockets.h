/**************************************************************************
*	SOCKETS.h
*	Librería de cierre y apertura de sockets. 
*	Autores:
*			Adrian Rubio
**************************************************************************/ 
#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <syslog.h>


/********
* FUNCIÓN: iniciar_servidor
* ARGS_IN: char * ip,int port,int clients, socklen_t * addrlen
* DESCRIPCIÓN: Inicia la configuracion basica para poner el servidor en escuha en un puerto tcp
				Contiene las llamadas a socket(),bind()ylisten()
* ARGS_OUT: status de le ejecution
********/
int iniciar_servidor(char *ip, int port,int clients, socklen_t * addrlen);
/********
* FUNCIÓN: cerrar_conexion
* ARGS_IN: int descriptor
* DESCRIPCIÓN: cierra la configuracion de su funcion hermana iniciar_servidor
* ARGS_OUT: 
********/
void cerrar_conexion(int descriptor);



#endif