/**************************************************************************
*	DAEMON.H
*	Libreria de demonizacion de programas
*	Autores:
*			Adrian Rubio
**************************************************************************/ 
 
#ifndef DAEMON_H
#define DAEMON_H

#include <syslog.h>
#include <unistd.h>
#include  <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <error.h>
#define MAXFD   64


/********
* FUNCIÓN: demonizar
* ARGS_IN: const char *pname, int facility
* DESCRIPCIÓN: demoniza cualquier programa que llam a dicha a la funcion
* ARGS_OUT: status de la ejecucion
********/
 int demonizar(const char *pname, int facility);



 #endif
