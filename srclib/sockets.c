
/**************************************************************************
*	SOCKETS.C
*	Librería de cierre y apertura de sockets. 
*	Autores:
*			Adrian Rubio
**************************************************************************/ 
#include "../includes/sockets.h"




/********
* FUNCIÓN: iniciar_servidor
* ARGS_IN: char * ip,int port,int clients, socklen_t * addrlen
* DESCRIPCIÓN: Inicia la configuracion basica para poner el servidor en escuha en un puerto tcp
				Contiene las llamadas a socket(),bind()ylisten()
* ARGS_OUT: status de le ejecution
********/

int iniciar_servidor(char * ip,int port,int clients, socklen_t * addrlen){
	/*ip,puerto,longtiud_dir*/
    int sockfd;
    struct sockaddr_in  self;
	syslog(LOG_INFO, "[sockets_lib]\n");

    // Creamos el socket tipo TCP */
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		syslog(LOG_ERR, "[sockets_lib]Error en la creación del socket\n");

		exit(errno);
	}

	// Inicializamos estructura de dirección y puerto
	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr= inet_addr(ip);
  
    syslog(LOG_INFO, "Bind socket");
    *addrlen = sizeof(self);
	// Ligamos puerto al socket
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		syslog(LOG_ERR, "[sockets_lib]In Socket binding\n");
		exit(errno);
	}

    syslog(LOG_INFO, "Escuchando conexiones...");
	// OK, listos para escuchar...
	if ( listen(sockfd, clients) != 0 )
	{
	
		syslog(LOG_ERR, "[sockets_lib]In Socket listening \n");
		exit(errno);
	}
	
	syslog(LOG_INFO, "Escuchando en [%s:%d]...\n", inet_ntoa(self.sin_addr), ntohs(self.sin_port));
    return sockfd;
}



/********
* FUNCIÓN: cerrar_conexion
* ARGS_IN: int descriptor
* DESCRIPCIÓN: cierra la configuracion de su funcion hermana iniciar_servidor
* ARGS_OUT: 
********/
void cerrar_conexion(int descriptor) {
	close(descriptor);
	syslog(LOG_INFO, "Cerrando conexion...\n");

}