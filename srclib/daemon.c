/**************************************************************************
*	DAEMON.C
*	Libreria de demonizacion de programas
*	Autores:
*			Adrian Rubio
**************************************************************************/ 
 
#include "../includes/daemon.h"


/********
* FUNCIÓN: demonizar
* ARGS_IN: const char *pname, int facility
* DESCRIPCIÓN: demoniza cualquier programa que llam a dicha a la funcion
* ARGS_OUT: status de la ejecucion
********/
 int demonizar(const char *pname, int facility) {
 	int   i;
 	pid_t pid;

     if ( (pid = fork()) < 0)
         return (-1);
     else if (pid) {
        syslog (LOG_ERR, "Error in fork");
        exit(0);               /* parent terminates */
     }

     /* child 1 continues... */

     umask(0);
     /* Ignorando demás señales de log */
     setlogmask (LOG_UPTO (LOG_INFO));
     /* Inicializando Log */
     openlog(pname, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);

     //syslog (LOG_INFO, "Initiating new server.");

     if (setsid() < 0)           /* become session leader */
         return (-1);

     signal(SIGHUP, SIG_IGN);

     if ( (pid = fork()) < 0)
         return (-1);
     else if (pid)
         exit(0);               /* child 1 terminates */

     /* child 2 continues... */


     //chdir("/");                 /* change working directory */

    

     syslog (LOG_INFO, "Closing standard file descriptors");
     syslog (LOG_INFO, "Initiating daemon process");

     /* Cerrando descriptores básicos */
     close(STDIN_FILENO); 
     close(STDOUT_FILENO); 
     close(STDERR_FILENO);


     return (0);                 /* success */
 }
