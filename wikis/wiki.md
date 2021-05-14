# WIKI - Servidor Web en C - Grupo 2361
### Adrián Rubio Pintado



## Introducción

    En esta prácticas se ha implementado un servidor Web sencillo en C. Entre las funcionalidades principales se destacan:
    
    
    1.  Servidor Web HTTP 1.0
    2.  Funcionamiento en modo demonio.
    3.  Soporte de verbos Get,Post y Options.
    4.  Loggeo de eventos con syslog.
    5.  Fichero de configuración de parámetros del servidor.
    6.  Soporte a la ejecución de scripts en Python y PHP.
    7.  Servidor concurrente basado en fork de procesos.
    8.  Soporte de respuestas: 404 Not Found,400 Bad Requesty 200 OK



## Compilación y Ejecucion

    Para crear los binarios solo hace falta llamar al comando make.
    Para ejecutar el programa bastará con un ./server , que ejecutará el programa en modo demonio.

## Desarrollo Técnico

        La funcionalidad del programa principal se encuentra en src/server.c.
        
        
###     I. Funcionalidad general y HTTP.

        La implementacion de esta funcionalidad de encuentra en src/server.c , srclib/http_lib y srclib/sockets.c. Se ha implementado un servidor estándar HTTP 1.0 (cierra la conexión después de
        enviar larespuesta) dado el uso que se le va a dar en la práctica, para liberar recursos y ganar en eficiencia y claridad de código.

        La librería de sockets se encarga de poner en escucha al servidor en el puerto deseado y bindear el socket de linux a dicho puerto. También se encarga de cerrar las conexiones. Para esta
        parte se han utilizado los sockets de linux.
        
        En server.c podemos ver la implementación que se encarga de dirigir la lógica de procesamiento del servidor según el verbo de la petición http, en concreto en la fución child_main, que es 
        la función llamada por cada proceso hijo del servidor concurrente, dicha función corresponde con la lógica de procesamiento de peticiones de un servidor iterativo. 
        
        En esta función primero se parsea la petición que llega por el socket. El parseo se hace con la libería externa PicoHttpParser. De aquí se obtiene el path del recurso solicitado, la 
        extensión, los argumentos que llegan por la url, etc. A partir de aqui se elige según el tipo de peticion que sea.
        
            1. Si es GET se comprueba si el recurso existe, si existe se abre y se envia.
            2. Tanto si es GET ,como si es POST, se comprueba si se solicita la ejecutcion de un script.
            3. Si es POST, se obtiene el body y se parsean los argumentos.
            4. Con cualquiea de los verbos de petición admitidos, se llama a responde_servidor(...), función de http_lib, encargada de formar las respuestas http correspondientes, segun los
               parametros pasados.


        En la librería http_lib, se forman las respuestas correspondientes que luego el servidor envía al cliente. A la función responde_servidor(...) de dicha librería, entre otros argumentos,
        se le pasa un buffer, que es rellenado en correspondencia con el código de respuesta y cabeceras de la respuesta que deba ser dada por el servidor. Dicho buffer es enviado después,desde
        server.c tras llamar a dicha función con send(). Una vez enviada la respuesta se cierra la conexión.


        La funcionalidad de respuesta http y de sockets  se compila primera como una librería propia en lib, y luego se linkea con el programa principal.


###     II. Funcionamiento en modo demonio.

        La implementacion de esta funcionalidad de encuentra en srclib/daemon.c.
        
        En esta librería se encuentra la función demonizar, que al ser llamada desde cualquier programa, lo demoniza, El programa principal (server), es lo primero que ejecuta.
        En este módulo, creamos un hijo 1, que primero ignora ciertas  señales del sistema con una máscara, inicializa luego  el logue y se convierta en el lider de la sesión(para que no
        haya problemas,ya que no se puede hacer con el proceso original que ha sido lanzado por el user). Ahora este proceso crea un proceso hijo(y él termina puesto que ya no es necesario)
        y es entonces cuando el hijo este cierra los descriptores ficheros y cambia el directorio de trabajo a la raíz(root), dado que este proceso ya está demonizado.

        Esta funcionalidad se compila primera como una librería propia en lib, y luego se linkea con el programa principal.


###     III.  Loggeo de eventos con syslog.
        
        Puesto que el servidor se ejecuta en modo demonio y se necesita dejar constancia de las peticiones y demás eventos de relevancia del servidor, se utiliza syslog. Se deja un mensaje de 
        log por cada petición recibida en el servidor, al inicializar el servidor o en caso de excepciones raras o que provocan la finalización de este(que en condiciones normales no deben 
        ocurrir), para saber en que estado se encuentra el servidor en todo momento.
        
        

###     IV.  Fichero de configuración de parámetros del servidor.

        La implementacion de esta funcionalidad de encuentra en srclib/config.c.

        Hay ciertos parámetros del servidor que se pueden configurar. Para cambiar dichos parámetros, basta con editar sus valores en el archivo de configuración server.conf que se encuentra en
        la raíz del proyecto. Los parámetros de cofiguración posibles(junto con sus valores por defecto) son:
        
        
        server_root = "WebFiles"   --> Raíz donde se deben de colocar los ficheros que sirva el servidor http
        max_clients = 10           --> Máximo numero de clientes simultaneos que acepta el servidor
        listen_port = 5555         --> Puerto de escucha a las peticiones http
        server_signature = "Servidor Web de Adri Rubio"  --> Firma de respuesta del servidor
        
        Para dicha implementación se ha utilizado la librería externa libconfuse. Para usar dicho servidor es necesario tenerla instalada en el sistema.
        La funcionalidad de configuración se compila primera como una librería propia en lib, y luego se linkea con el programa principal.



###    VI.Soporte a la ejecución de scripts en Python y PHP.

        La implementacion de esta funcionalidad de encuentra en srclib/cgi.c. Los scripts que se utilicen deben de estar situados en la carpeta scripts(ptyhon y php)
        
        Para la implementación de dicha característica se han usado dos procesos comunicados con 2 tuberias. El proceso hijo,
        en la tubería input_script clona el descriptor de lectura con stdin, y en la tubería output_script, clona el descriptor de escritritura con stdout. Es este porceso el que ejecuta el 
        scripty sus comandos. El proceso padre le dice al hijo por la tuberia input script, que comando quiere que ejecute,dicho comando es recogido del server por la petición http. El
        proceso padre recoge el resultado de ejecución, que finalmente se le envia al cliente por http en modod de texto plano. 
        
        Solo se soportan scripts en python o php. Para python se ha utilizado la versión 3 del intérprete. Además solo se recogerán 4096 bytes de salida de la ejecución,para ganar en eficiencia 
        y en tiempo de respuesta.
        
        Toda esta funcionalidad se compila primera como una librería propia en lib, y luego se linkea con el programa principal.
        
        
        
###     VII.Servidor concurrente basado en fork de procesos.  

        Para hacer el servidor concurrente se ha elegido finalmente hacer un pool de procesos. Dada la facilidad de los procesos para heredar los descriptores del proceso padre se ha elegido
        esta opción. En server.c se puede obervar como se bindea el socket a un puerto y luego se hace el pool de procesos con child_make(). Se crean tantos procesos en el pool como se indiquen 
        con el parámetro max_clients en el archivo de configuración, dado que solo se pueden atender a esos clientes simultáneamente, puesto que cada proceso equivale a "un servidor iterativo".
        Otra de las razones por las que se eligió el pool de procesos es el hecho de que si se elige un número del pool óptimo, se puede conseguir un equilibrio más que razonable entre la carga 
        del servidor y el número de peticiones simultaneas reales que se esperan en el servidor,con un servidor bastante eficiente.

## Conclusiones Técnicas

    La implementación en C supone un mayor control sobre los datos a bajo nivel, pero también más trabajo y esfuerzo. Una gran parte del trabajo ha sido el correcto parseo y tratado de las 
    cadenas de texto entre los diferentes módulos del programa. Esto es desde correcto parseo de la url con todas sus posibles opciones hasta la recogida de cabeceras. Afortunadamente algunas 
    partes, como la última se hacen con el uso de librerías externas ya implementadas. También la lectura de parámetros de configuración del servidor.
    
    La otra dificultad añadida es la correcta llamada del programa a las funciones del sistema operativo para tareas como demonizar,abrir y cerrar sockets en el momento necesario o la 
    ejecucion de scripts de la manera deseada.


## Conclusiones Personales

    Personalmente creo que es una práctica sencilla a nivel conceptual, pero requiere una gran atención en la implementación en un leguaje como C, dado que requiere de un "control manual" de 
    los recursos del SO, y hay que prestar especial atención al orden en el que se llaman y a cómo interactúan entre ellos. Gracias a ello se obtiene un conocimiento muy práctico y se entienden
    mejor los diferentes problemas o retos que acarrean implementar un servidor web, que si dicha implementación se hubiera requerido en otro lenguaje como python.
