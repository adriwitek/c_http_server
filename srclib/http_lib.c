/**************************************************************************
*	HTTP_LIB.C
*	Libreria de  RESOLUCION DE PETICIONES HTTP
*	Autores:
*			Adrian Rubio
**************************************************************************/ 

#include "../includes/http_lib.h"


/********
* FUNCIÓN: responde_servidor
* ARGS_IN: char * buf_res, int numero_respuesta,int minor_version,char * tipo_recurso,int tam, int flag,char* last_modified
* DESCRIPCIÓN: funcion para el envio rapido de respuestas de servidor HTTP
* ARGS_OUT: 
********/
void responde_servidor(char * buf_res, int numero_respuesta,int minor_version,char * tipo_recurso,int tam, int flag,char* last_modified){

/* FUNCIONES PARA TIEMPO DE CABECERAS */
		time_t now;
		struct tm *tm;
        char  fecha[150];
        char aux[1000];

        syslog(LOG_INFO, "Uso de la libreria httplib\n");

				/* Versión de HTTP  */
				sprintf(buf_res, "HTTP/1.%d ", minor_version);
            
				/* STATUS */
                switch (numero_respuesta)
                {
                    case OK:
                        
                        strcat(buf_res , "200 OK\r\n");
                        /* FECHA */
                        now = time(NULL);
                        tm = gmtime(&now);
			        	strcat(buf_res, "Date: ");
			        	strftime(fecha, 100, "%a,  %d %b %Y, %H:%M:%S GMT\r\n", tm);
			        	strcat(buf_res, fecha);
  
                        if(flag == 1) {
                            strcat(buf_res, "Allow: GET, OPTIONS, POST\r\n");
                        }
                        /*Last Modified*/
                        if(last_modified != NULL){
                            strcat(buf_res, "Last-Modified: ");
                            strcat(buf_res,last_modified);
                            strcat(buf_res,"\r");
                        }
                        /* Tipo  */
                        if(tipo_recurso != NULL){
                            strcat(buf_res, "Content-Type: ");
                            strcat(buf_res, tipo_recurso);
                            strcat(buf_res, "\r\n");
                        }
				   
				        /* Tamaño  */
				        strcat(buf_res, "Content-Lenght: ");
                        sprintf(aux, "%d\r\n", tam);
                        strcat(buf_res, aux);
                        /*Close connection*/
                        strcat(buf_res, "Connection: close \r\n");
                        /* Firma de Servidor */
			        	strcat(buf_res, "Server: Servidor2361_19\r\n\r\n");
                        //printf("\n El contenido de el buffer  OKes :\n%s",buf_res);

                        break;
                    case BAD_REQUEST:
                        strcat(buf_res , "400 Bad Request\r\n");
                        /* FECHA */
                        now = time(NULL);
                        tm = gmtime(&now);
			        	strcat(buf_res, "Date: ");
			        	strftime(fecha, 100, "%a,  %d %b %Y, %H:%M:%S GMT\r\n", tm);
			        	strcat(buf_res, fecha);
                        strcat(buf_res, "Content-Length: 0\r\n");
                        strcat(buf_res, "Content-Type: text/plain\r\n");
                        /*Close connection*/
                        strcat(buf_res, "Connection: close\r\n");
			        	/* Firma de Servidor */
			        	strcat(buf_res, "Server: Servidor2361_19\r\n\r\n");
                        //printf("\n El contenido de el buffer  BAD REQUESTes :\n%s",buf_res);
                        syslog(LOG_ERR, "[httplib]: Bad Request\n");

                        break;
                    case NOT_FOUND:
                        strcat(buf_res , "404 Not Found\r\n");
                        /* FECHA */
                        now = time(NULL);
                        tm = gmtime(&now);
			        	strcat(buf_res, "Date: ");
			        	strftime(fecha, 100, "%a,  %d %b %Y, %H:%M:%S GMT \r\n", tm);
			        	strcat(buf_res, fecha);
                        /*Close connection*/
                        strcat(buf_res, "Connection: close\r\n");
			        	/* Firma de Servidor */
			        	strcat(buf_res, "Server: Servidor2361_19\r\n\r\n");
                        syslog(LOG_ERR, "[httplib] : 404 Recurso no encontrado\n");

                        break; 
                    default://duplicado
               
                        strcat(buf_res , "404 Not Found\r\n");
                        /* FECHA */
                        now = time(NULL);
                        tm = gmtime(&now);
			        	strcat(buf_res, "Date:");
			        	strftime(fecha, 100, "%a,  %d %b %Y, %H:%M:%S GMT\r\n", tm);
			        	strcat(buf_res, fecha);
                        /*Close connection*/
                        strcat(buf_res, "Connection: close\r\n");
			        	/* Firma de Servidor */
			        	strcat(buf_res, "Server: Servidor2361_19\r\n\r\n");
                        syslog(LOG_ERR, "[httplib]: 404 Recurso no encontrado\n");
                        break;
                }


				
                
     }

