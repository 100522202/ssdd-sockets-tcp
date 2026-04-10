#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <pthread.h>
#include "claves.h"
#include "mensajes.h"
#include "procesar_funcion.h"


#define NUMBER_OF_PORTS 65535

// Mutex y cond var globales para proteger socket_especifico_fd
static pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
static int leyendo = 0; // 1 = algún hijo leyendo el fd, 0 libre

// Función para procesar las peticiones de los hilos
void *procesar_peticion(void* socket_especifico_fd){
    
    // Bloquear antes de leer el fd
    pthread_mutex_lock(&mutex_socket);
    
    // Leer el fd e indicar que ya no está leyendo
    int fd_local = *(int*)socket_especifico_fd;

    leyendo = 0;
    // Avisar al hilo padre (servidor)
    pthread_cond_signal(&cond_var);

    pthread_mutex_unlock(&mutex_socket);

    // ---- Tratamiento de la petición ----

    // Leer el cod_op
    unsigned char codigo_operacion;

    if (recvMessage(fd_local, &codigo_operacion, sizeof(codigo_operacion)) < 0){
        perror("recvMessage error leyendo el cod_op");
        pthread_exit(NULL);
    }

    // Procesar según qué operación sea

    switch (codigo_operacion) {
    case OP_SET:
        procesar_set(fd_local);
        break;

    case OP_GET:
        procesar_get(fd_local);
        break;

    case OP_MODIFY:
        procesar_modify(fd_local);
        break;

    case OP_DELETE:
        procesar_delete(fd_local);
        break;

    case OP_EXIST:
        procesar_exist(fd_local);
        break;

    case OP_DESTROY:
        procesar_destroy(fd_local);
        break;

    default:
        // TODO: Código de operación desconocido: mandar error al cliente
        
        break;
    }
    pthread_exit(NULL);
}

int main(int argc, char * argv[]){
    
    if (argc != 2){
        printf("Error en el número de argumentos pasados a servidor\n");
        return -1;
    }

    // Capturar el número de puerto, strtol para robustez

    char * endptr;
    errno = 0;
    long puerto = strtol(argv[1], &endptr, 10);

    // Validar formato correcto
    if ((errno == ERANGE && (puerto == LONG_MAX || puerto == LONG_MIN)) || (errno != 0 && puerto == 0)) {
        perror("strtol");
        return -1;
    }

    if (puerto > NUMBER_OF_PORTS){
        printf("Error: Puerto %ld mayor que %d\n", puerto, NUMBER_OF_PORTS);
        return -1;
    }

    if (puerto < 0){
        printf("Error: Puerto %ld menor que 0\n", puerto);
        return -1;
    }

    if (endptr == argv[1]){
        printf("Error: Puerto no es un número\n");
        return -1;
    }
    
    if (*endptr != '\0'){
        printf("Error: Puerto contiene caracteres inválidos\n");
        return -1;
    }

    // Crear la dirección del socket iniciada a 0
    struct sockaddr_in socket_servidor_addr;
    memset(&socket_servidor_addr, 0, sizeof(socket_servidor_addr));

    // Rellenar los atributos
    socket_servidor_addr.sin_family = AF_INET;
    // Convertir host to network, 16 bits -> short (los datos viajarán por la red)
    socket_servidor_addr.sin_port = htons((uint16_t)puerto);
    socket_servidor_addr.sin_addr.s_addr = INADDR_ANY;

    // Crear descriptor del socket
    int socket_servidor_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_servidor_fd < 0){
        perror("socket");
        return -1;
    }
    // TODO_ meter reuse addrs a la hora de crear todos los sockets
    // Unir addr y fd
    if (bind(socket_servidor_fd, (struct sockaddr *)&socket_servidor_addr, sizeof(socket_servidor_addr)) < 0){
        perror("bind");
        close(socket_servidor_fd);
        return -1;
    }

    // Poner el socket en escucha
    if (listen(socket_servidor_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(socket_servidor_fd);
        return -1;
    }

    // Si se llega hasta aquí es que el servidor está escuchando
    printf("Servidor escuchando en el puerto %ld...\n", puerto);

    //Bucle principal del servidor
    while (1){

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Socket nuevo para cada cliente

        int socket_especifico_fd = accept(socket_servidor_fd, (struct sockaddr *)&client_addr,  &client_len);
        
        // Si hay un fallo se lanza error y se continúa escuchando

        if (socket_especifico_fd < 0){
            perror("accept");
            continue;
        }


        // Crear un hilo para procesar cada solicitud
        
        pthread_t id_hilo;
        pthread_attr_t attr_hilo;

        pthread_attr_init(&attr_hilo);
        pthread_attr_setdetachstate(&attr_hilo, PTHREAD_CREATE_DETACHED);
        
        pthread_create(&id_hilo, &attr_hilo, procesar_peticion, (void *)&socket_especifico_fd);
        pthread_attr_destroy(&attr_hilo);

        // Proteger el fd (el padre servidor podría sobrescribir antes de que lo lea un hijo cliente)
        pthread_mutex_lock(&mutex_socket);
        
        while (leyendo == 1){
            // Esperar mientras haya un hilo copiando el fd (cuando termine será 0)
            pthread_cond_wait(&cond_var, &mutex_socket);
        }
        
        // Para que cuando lance al próximo hilo cliente tenga que esperar a que lea su fd
        leyendo = 1;
        pthread_mutex_unlock(&mutex_socket);
    }

    return 0;
}