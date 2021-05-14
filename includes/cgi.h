/**************************************************************************
*	CGI.H
*	Libreria de ejecucion de Scripts por entrada estandar
*	Autores:
*			Adrian Rubio
**************************************************************************/
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include <syslog.h>

#define MAX_SCRIPT_OUTPUT_SIZE 4096
#define NO_SCRIPT 0
#define PYTHON_SCRIPT 1
#define PHP_SCRIPT 2
#define MAX_INTERPRETERNAME_LEN 50

/********
* FUNCIÓN: run_script
* ARGS_IN: int script_code,char ** argumentos,int n_argumentos, char* output
* DESCRIPCIÓN: Ejecuta un script en python o php, en la carpeta de scripts,pasandole la onfo por entrada sttandarç
				Tambien simula la salida del script en ejecucion con salida estandar
				el ultimo en los arrays del argumento, tiene que ser NULL
*
* ARGS_OUT: status de le ejecution
********/
int run_script(int script_code ,char* resource,char ** argumentos,int n_argumentos, char* output);
