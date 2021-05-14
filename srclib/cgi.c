/**************************************************************************
*	CGI.C
*	Libreria de ejecucion de Scripts por entrada estandar
*	Autores:
*			Adrian Rubio
**************************************************************************/

#include <stdlib.h>
#include <syslog.h>

#include "../includes/cgi.h"




/********
* FUNCIÓN: run_script
* ARGS_IN: int script_code,char ** argumentos,int n_argumentos, char* output
* DESCRIPCIÓN: Ejecuta un script en python o php, en la carpeta de scripts,pasandole la onfo por entrada sttandarç
				Tambien simula la salida del script en ejecucion con salida estandar
				el ultimo en los arrays del argumento, tiene que ser NULL
*
* ARGS_OUT: status de le ejecution
********/
int run_script(int script_code,char* resource,char ** argumentos,int n_argumentos, char* output){
 
 	//printf("aqui comieza la prueba \n");
 	//char doc[100];
 	//FILE *f = popen(comando_completo,"r");
 	//fread(doc,100,1,f);
 	//printf("hemos llegado : %s\n",doc);


	
	int input_script[2],output_script[2],readbytes;
	char script_interpreter[MAX_INTERPRETERNAME_LEN];
	pid_t pid;
	int readed_bytes=0;
	pipe(input_script);
	pipe(output_script);
	//char command[50] = "python3";
	//char * args_child[2];
	//args_child[0] = (char *)calloc(1,50);
	//(args_child[0] == NULL ){
	//    return -1;
	//}

	//sprintf(args_child[0],"abbb");
	
	if(script_code == PYTHON_SCRIPT){
		strcpy(script_interpreter, "/usr/bin/python3");
	}else if(script_code == PHP_SCRIPT){
		strcpy(script_interpreter, "/usr/bin/php");
	}else{
		syslog(LOG_ERR, "[cgi_module]ERROR Tipo de script no soportado\n");
		return -1;
	}



	 if( (pid=fork())  == 0){
		 /*hijo*/
		 /*clonamos stdin*/
		close(input_script[1]);//cerramos escritura
		//dup2(input_script[0],STDIN_FILENO);//clonamos la entrada estandar
		dup2(input_script[0],fileno(stdin));//clonamos la entrada estandar
		close( input_script[0] );


		/*clonamos stdout*/
		close(output_script[0]);//cerramos lectura
		//dup2(output_script[1],STDOUT_FILENO);
		dup2(output_script[1],fileno(stdout));
		close(output_script[1]);
		/*ejecutamos script*/
		//execvp("echo", args_child);/*script a ejecutar*/
		//execl("/usr/bin/python3","scripts/test.py",NULL);
		execl(script_interpreter, "cgi_server", resource, (char *) NULL);
		fflush(NULL);
	 }else{
		 /*padre*/

		close(input_script[0]);//cerramos lectura
		close(output_script[1]);//cerramos escritura

		//dup2(input_script[1],STDOUT_FILENO);


		for(int i = 0;i<n_argumentos;i++){
			strcat(argumentos[i], "  ");
			write(input_script[1], argumentos[i], strlen(argumentos[i]));
			//printf("\n\nargu:%s",argumentos[i]);
			//fflush(NULL);
		}
		close(input_script[1]);

		/*Recogemos la salida del script por la tuberia output_script*/
		wait(NULL);
		readed_bytes=read(output_script[0],output,MAX_SCRIPT_OUTPUT_SIZE);
		
		close(output_script[0]);
		syslog(LOG_INFO, "[cgi_module]Solucitud procesada correctamente\n");
		return readed_bytes;
	 }
 	
 } 

