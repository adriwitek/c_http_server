/**************************************************************************
*	SERVIDOR CON FUNCIONAMIENTO HTTP
*	Implementacion de un server en c: Programa Principal
*	Autores:
*			Adrian Rubio
**************************************************************************/
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "../includes/cgi.h"
#include "../includes/config.h"
#include "../includes/daemon.h"
#include "../includes/http_lib.h"
#include "../includes/picohttpparser.h"
#include "../includes/sockets.h"

/*TAm. max. del buffer de la cabecera de una peticion HTTP*/
#define MAX_BUFFER 4096
/*Numero max. de argumentos que se pueden leer de una peticion HTTP*/
#define MAX_NUMBER_OF_ARGUMENTS 100
/*Tam max. de cada argumento pasado al cgi*/
#define MAX_ARG_SIZE 150

static int nchildren;
static pid_t *pids;
pid_t child_make(int nchild, int sockfd);
void child_main(int nchild, int sockfd);

/*Default maxchilds config*/
int MAXCHILD = 5;

char* SERVER_ROOT ;



/********
* FUNCIÓN: sig_int
* ARGS_IN: int sign
* DESCRIPCIÓN: hace al padre esperar por os hijos
* ARGS_OUT: 
********/
void sig_int(int sign) {
    int i;

    for (i = 0; i < MAXCHILD; i++) {
        kill(pids[i], SIGTERM);
    }

    while (wait(NULL) > 0)
        ; /* Hacemos esperar a los hijos */

    exit(0);
}

/********
* FUNCIÓN: main
* ARGS_IN: 
* DESCRIPCIÓN: funcionalidad principal del servidor, instancia la configuracion y atiende peticiones http
* ARGS_OUT: -1 en caso de error
********/
int main(int argc, char **argv) {
    int sockfd, childpid;
    struct sockaddr_in self;
    struct hostent *sp;
    socklen_t addrlen;
    char dir_actual[1024];
    Config config; /* Estructura para fichero de configuración */

    if (getcwd(dir_actual, sizeof(dir_actual)) == NULL) {
        syslog(LOG_ERR, "ERROR al conseguir PATH\n");
        exit(0);
    }

    /* Funcion de DEMONIZAR *//*
    demonizar(argv[0], 0);
    syslog(LOG_INFO, "Servidor demonizado");*/

    syslog(LOG_INFO, "Iniciando configuracion del servidor");
    /* Inicializamos servidor con su configuración */
    config = config_server("server.conf");
    sockfd = iniciar_servidor("127.0.0.1", config.server_port, config.clients, &addrlen);

    /* Funcion de DEMONIZAR */
    demonizar(argv[0], 0);
    syslog(LOG_INFO, "Servidor demonizado");

	SERVER_ROOT = strdup(config.server_root);
    syslog(LOG_INFO, "Servidor configurado. Escuchando en 127.0.0.1:%d", config.server_port);
    //syslog(LOG_INFO, "Servidor escuchando ");

    MAXCHILD = config.clients;
    pids = (pid_t *)calloc(MAXCHILD, sizeof(pid_t));

    for (int i = 0; i < MAXCHILD; i++)
        pids[i] = child_make(i, sockfd); /* Esta funcion nunca retorna */

    signal(SIGINT, sig_int);

    /* Ponemos al padre en espera y el trabajo lo realizan los hijos */
    while (1) {
        pause();
    }

    /* Cerramos conexiones de socket */
    cerrar_conexion(sockfd);
    syslog(LOG_INFO, "Apagando el servidor...\n");
    return 0;
}

/********
* FUNCIÓN: child_main
* ARGS_IN: int nchild, int sockfd
* DESCRIPCIÓN: dado un sockect y el numero de "ejecuciones", reponde a peticones http
*				La funcionalidad de esta aplicacion es la misma que la que tendria un servidor iteracivo(1 instacia en ejecucion)
* ARGS_OUT: 
********/
void child_main(int nchild, int sockfd) {
    /* ****SERVIDOR ITERATIVO *****/
    while (1) {
        int clientfd;
        struct sockaddr_in client_addr;
        char buf[4096], tipo_recurso[25], *cabecera, *fichero_buf, path_copy[1024], path_copy_2[1024];
        char * script_path;
        char* complete_path;
        char *pos;
        char *extension, *argumentos_cgi = NULL, *cadena_aux, *cadena_aux2;
        char *argumentos_cgi_formateados[100];
        int num_argumentos_formateados = 0;
        const char *path, *method;
        int pret, minor_version;
        struct phr_header headers[100];
        size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
        int tam_fichero = 0, i, file;
        int script_flag = NO_SCRIPT;
        char content_l_aux[15];
        int content_lenght = -1;
        int sum = 0;
        ssize_t rret;
        FILE *documento = NULL;

        socklen_t addrlen = sizeof(client_addr);

        /* Reservas de memoria de buffers */
        //tipo_recurso = (char *) malloc(MAX_BUFFER* sizeof(char));
        cabecera = (char *)malloc(MAX_BUFFER * sizeof(char));

        if (tipo_recurso == NULL) {
            syslog(LOG_ERR, "Error de memoria\n");
            exit(0);
        }

        for (int i = 0, j = 0; i < MAX_NUMBER_OF_ARGUMENTS; i++) {
            argumentos_cgi_formateados[i] = (char *)calloc(1, MAX_ARG_SIZE);
            j++;
            if (argumentos_cgi_formateados[i] == NULL) {
                for (int k = 0; k < j; k++) {
                    free(argumentos_cgi_formateados[k]);
                }
                return;
            }
        }

        /* Quedamos a la espera de conexiones */
        clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
        syslog(LOG_INFO, "Conexión desde [%s:%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* Funcionalidad HTTP */
        while ((rret = read(clientfd, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR);
        
        pos = buf + buflen;
        prevbuflen = buflen;
        buflen += rret;
        /* Contamos el número de headers de la cabecera */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        /* Parseamos la petición*/
        pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
                                 &minor_version, headers, &num_headers, prevbuflen);

        syslog(LOG_INFO, "Petición http parseada\n");

        if (!(pret < 0)) { /*En ocasiones phr_parse devueve numero negativos si ha habido errores*/

          
            syslog(LOG_INFO, "Path solicitado:%.*s\n\n", (int)path_len, path);


            /* Conseguimos el path */
            sprintf(path_copy, "%.*s", (int)path_len, path);

            /*Conseguimos la extension*/
            extension = strrchr(path_copy, '.');

            if (extension != NULL) {
                cadena_aux2 = extension++;
                cadena_aux = strtok(extension, "?");
                if (cadena_aux != NULL) {
                    extension = cadena_aux;
                }
                /*argumentos '?' despues de extension*/
                sprintf(path_copy_2, "%.*s", (int)path_len, path);
                argumentos_cgi = strchr(path_copy_2, '?');

                if (argumentos_cgi != NULL) {
                    argumentos_cgi++;
                    /*formateamos los argumentos del body, para pasarselos al script*/
                    cadena_aux = strtok(argumentos_cgi, "&");
                    if (cadena_aux != NULL) {
                        while (cadena_aux != NULL && num_argumentos_formateados < MAX_NUMBER_OF_ARGUMENTS) {
                            sprintf(argumentos_cgi_formateados[num_argumentos_formateados], "%s", cadena_aux);
                            num_argumentos_formateados++;
                            cadena_aux = strtok(NULL, "&");
                        }
                    }
                }
            } else { /*argumentos en el path pero no hay extension*/
                argumentos_cgi = strchr(path_copy, '?');
                if (argumentos_cgi != NULL) {
                    /*formateamos los argumentos del body, para pasarselos al script*/
                    cadena_aux = strtok(argumentos_cgi, "&");
                    if (cadena_aux != NULL) {
                        while (cadena_aux != NULL && num_argumentos_formateados < MAX_NUMBER_OF_ARGUMENTS) {
                            sprintf(argumentos_cgi_formateados[num_argumentos_formateados], "%s", cadena_aux);
                            num_argumentos_formateados++;
                            cadena_aux = strtok(NULL, "&");
                        }
                    }
                }
            }

            //OBTENEMOS EL PATHCOMPLETO Y EL TIPO DE RECURSO
            complete_path = malloc(strlen(SERVER_ROOT) + strlen(path_copy) + 1);
            strcpy(complete_path, SERVER_ROOT);

            if (strcmp(path_copy, "/") == 0) {
                strcpy(tipo_recurso, "text/html; charset=UTF-8");
                strcat(complete_path, "/index.html");

            } else if (extension != NULL) {
                strcat(complete_path, path_copy);
                if (strncmp(extension, "txt", 3) == 0) {
                    strcpy(tipo_recurso, "text/plain");
                } else if (strncmp(extension, "html", 4) == 0) {
                    strcpy(tipo_recurso, "text/html; charset=UTF-8");
                } else if (strncmp(extension, "htm", 3) == 0) {
                    strcpy(tipo_recurso, "text/html; charset=UTF-8");
                } else if (strncmp(extension, "gif", 3) == 0) {
                    strcpy(tipo_recurso, "image/gif");
                } else if (strncmp(extension, "jpg", 3) == 0) {
                    strcpy(tipo_recurso, "image/jpeg");
                } else if (strncmp(extension, "jpeg", 4) == 0) {
                    strcpy(tipo_recurso, "image/jpeg");
                } else if (strncmp(extension, "mpg", 3) == 0) {
                    strcpy(tipo_recurso, "video/mpeg");
                } else if (strncmp(extension, "mpeg", 4) == 0) {
                    strcpy(tipo_recurso, "video/mpeg");
                } else if (strncmp(extension, "doc", 3) == 0) {
                    strcpy(tipo_recurso, "video/msword");
                } else if (strncmp(extension, "docx", 4) == 0) {
                    strcpy(tipo_recurso, "video/msword");
                } else if (strncmp(extension, "pdf", 3) == 0) {
                    strcpy(tipo_recurso, "application/pdf");
                    /*soporte de scripts*/
                } else if (strncmp(extension, "py", 3) == 0) {
                    script_path = path_copy;
                    script_path++;
                    script_flag = PYTHON_SCRIPT;
                    strcpy(tipo_recurso, "text/plain");
                } else if (strncmp(extension, "php", 3) == 0) {
                    script_path = path_copy;
                    script_path++;
                    script_flag = PHP_SCRIPT;
                    strcpy(tipo_recurso, "text/plain");
                } else { /*Tipo de Fichero NO Soportado */
                    strcpy(tipo_recurso, "nula");
                    syslog(LOG_ERR, "Tipo nulo de recurso solicitado\n");
                }
                
            }

            /********************TIPO DE SOLICITUD**************/

            /* METODO GET */
            if (strncmp(method, "GET ", 4) == 0) {

                if (strncmp(tipo_recurso, "nula", 4) == 0) { /*Tipo de Fichero NO Soportado */
                    responde_servidor(cabecera, BAD_REQUEST, minor_version, tipo_recurso, tam_fichero, 0, NULL);
                    send(clientfd, cabecera, strlen(cabecera), 0);

                } else if (script_flag == PYTHON_SCRIPT || script_flag == PHP_SCRIPT) {

                    /*ejecucion de cgis por get*/
                    char script_output[MAX_SCRIPT_OUTPUT_SIZE * 4];
                    int n_bytes_leidos;
                    time_t now;
                    char fecha[150];

                    



                    n_bytes_leidos = run_script(script_flag,script_path, argumentos_cgi_formateados, num_argumentos_formateados, script_output);
                    if (n_bytes_leidos < 0) {
                        syslog(LOG_ERR, "Salida vacia del script procesado\n");
                    }

                    /*send result*/
                    tam_fichero = n_bytes_leidos;

                    /*Ultima fecha de modificacion*/
                    strftime(fecha, 100, "%a,  %d %b %Y, %H:%M:%S GMT\r\n", gmtime(&now));

                    /* Montamos y enviamos cabecera */
                    strcpy(tipo_recurso, "text/plain");
                    responde_servidor(cabecera, OK, minor_version, tipo_recurso, tam_fichero, 0, fecha);
                    send(clientfd, cabecera, strlen(cabecera), MSG_MORE);

                    /* Enviamos fichero al cliente */
                    send(clientfd, script_output, tam_fichero, 0);

                } else if ((documento = fopen(complete_path, "r")) == NULL) { /*Fichero no encontrado */
                    responde_servidor(cabecera, NOT_FOUND, minor_version, NULL, tam_fichero, 0, NULL);
                    send(clientfd, cabecera, strlen(cabecera), 0);

                } else {
                   
                    /* Conseguimos tamaño del fichero */
                    if ((fseek(documento, 0, SEEK_END)) < 0) {
                        syslog(LOG_ERR, "ERROR en fseek\n");
                        fclose(documento);
                        exit(0);
                    }
                    tam_fichero = ftell(documento);
                    fseek(documento, 0, SEEK_SET);

                    /*Ultima fecha de modificacion*/
                    struct stat attributos_fichero;
                    stat(complete_path + 1, &attributos_fichero);

                    /* Montamos y enviamos cabecera */
                    responde_servidor(cabecera, OK, minor_version, tipo_recurso, tam_fichero, 0, ctime(&attributos_fichero.st_mtime));
                    send(clientfd, cabecera, strlen(cabecera), MSG_MORE);


                    /* Enviamos fichero al cliente */
                    fichero_buf = (void *)calloc(tam_fichero, sizeof(void));
                    fread(fichero_buf, tam_fichero, 1, documento);
                    send(clientfd, fichero_buf, tam_fichero, 0);
                    fclose(documento);
                }




            } else if (strncmp(method, "POST ", 4) == 0) { /* *************MÉTODO POST **************/

                time_t now;
                char script_output[MAX_SCRIPT_OUTPUT_SIZE * 4];
                char *script_out;
                char fecha[150];
                int n_bytes_leidos;
                char *body_data;
                char *cadena_aux_3;
                int nm_argumentos_formateados = 0;
                char *args_cgi_formateados[100];
                char contenido[MAX_SCRIPT_OUTPUT_SIZE];

                /*Conseguimos el data del body de la request*/
                /*get content leght*/
                for (size_t i = 0; i < num_headers; i++) {
                    sum += headers[i].name_len + headers[i].value_len;
                    sprintf(content_l_aux, "%.*s", (int)headers[i].name_len, headers[i].name);
                    if (strcmp("Content-Length", content_l_aux) == 0) {
                        content_lenght = atoi(headers[i].value);
                    }
                }

                /*Conseguimos el data del body de la request*/
                body_data = strstr(buf, "\r\n\r\n");
                body_data += 4;
                strncpy(contenido, body_data, content_lenght);
                contenido[content_lenght] = '\0';

                for (int i = 0, j = 0; i < MAX_NUMBER_OF_ARGUMENTS; i++) {
                    args_cgi_formateados[i] = (char *)calloc(1, 100);
                    j++;
                    if (args_cgi_formateados[i] == NULL) {
                        syslog(LOG_ERR, "Error en la reserva de memoria\n");
                        for (int k = 0; k < j; k++) {
                            free(args_cgi_formateados[k]);
                        }
                        return;
                    }
                }

                /*formateamos los argumentos del body, para pasarselos al script*/
                cadena_aux_3 = strtok(contenido, "&");
                if (cadena_aux_3 != NULL) {
                    while (cadena_aux_3 != NULL && nm_argumentos_formateados < MAX_NUMBER_OF_ARGUMENTS) {
                        sprintf(args_cgi_formateados[nm_argumentos_formateados], "%s", cadena_aux_3);
                        nm_argumentos_formateados++;
                        cadena_aux_3 = strtok(NULL, "&");
                    }
                }

             
                /*runscript*/
                n_bytes_leidos = run_script(script_flag, script_path,args_cgi_formateados, nm_argumentos_formateados, script_output);
                if (n_bytes_leidos < 0) {
                    syslog(LOG_ERR, "Salida vacia del script procesado\n");
                }

                /*send result*/
                tam_fichero = n_bytes_leidos;

                /*Ultima fecha de modificacion*/
                strftime(fecha, 100, "%a,  %d %b %Y, %H:%M:%S GMT\r\n", gmtime(&now));

                /* Montamos y enviamos cabecera */
                strcpy(tipo_recurso, "text/plain");
                responde_servidor(cabecera, OK, minor_version, tipo_recurso, tam_fichero, 0, fecha);
                send(clientfd, cabecera, strlen(cabecera), MSG_MORE);

                /* Enviamos fichero al cliente */
                send(clientfd, script_output, tam_fichero, 0);

            } else if (strncmp(method, "OPTIONS ", 4) == 0) { /***************** MÉTODO OPTIONS ************/
                responde_servidor(cabecera, OK, minor_version, NULL, tam_fichero, 1, NULL);
                send(clientfd, cabecera, strlen(cabecera), 0);
            }

            /* Cerramos la conexión del socket cliente */
            free(complete_path);
            free(cabecera);
            cerrar_conexion(clientfd);
        }
    }
}

/********
* FUNCIÓN: child_make
* ARGS_IN: int nchild, int sockfd
* DESCRIPCIÓN: dado un sockect y el numero max de hijos(procesamiento concurrente), crea las diferentes instacias para 
*				crear procesos paralelos con la misma fucnionalidad:servidor http		
* ARGS_OUT: 
********/
pid_t child_make(int nchild, int sockfd) {
    pid_t pid;

    /* Código del padre */
    if ((pid = fork()) > 0)
        return (pid);

    child_main(nchild, sockfd);
}
