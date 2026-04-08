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
#include "claves.h"
#include "mensajes.h"


#define NUMBER_OF_PORTS 65535

// Función auxiliar para procesar la operación exist
int procesar_exist(int socket_especifico_fd) {
    /* 1. Leer la longitud de la clave */
    int32_t longitud_clave_red;

    if (recvMessage(socket_especifico_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        perror("recvMessage longitud_clave_red");
        return -1;
    }

    /* 2. Pasar la longitud de formato de red a formato de máquina */
    int32_t longitud_clave = ntohl(longitud_clave_red);

    printf("Longitud de clave recibida: %d\n", longitud_clave);

    /* 3. Validar la longitud */
    if (longitud_clave < 0 || longitud_clave > 255) {
        int32_t resultado_error = htonl(-1);

        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error longitud");
            return -1;
        }

        return 0;
    }

    /* 4. Leer la clave */
    char clave[256];

    if (recvMessage(socket_especifico_fd, clave, (size_t)longitud_clave) < 0) {
        perror("recvMessage clave");
        return -1;
    }

    /* 5. Añadir fin de cadena para usarla como string de C */
    clave[longitud_clave] = '\0';

    printf("Clave recibida: %s\n", clave);

    /* 6. Llamar a la lógica real */
    int resultado_exist = exist(clave);

    printf("Resultado de exist(\"%s\") = %d\n", clave, resultado_exist);

    /* 7. Convertir el resultado a formato de red */
    int32_t resultado_exist_red = htonl(resultado_exist);

    /* 8. Enviar respuesta al cliente */
    if (sendMessage(socket_especifico_fd, &resultado_exist_red, sizeof(resultado_exist_red)) < 0) {
        perror("sendMessage resultado_exist_red");
        return -1;
    }

    return 0;
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

    // Unir addr y fd
    if (bind(socket_servidor_fd, (struct sockaddr *)&socket_servidor_addr, sizeof(socket_servidor_addr)) < 0){
        perror("bind");
        close(socket_servidor_fd);
        return -1;
    }

    // poner el socket en escucha
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
        
        //Socket nuevo para cada cliente
        int socket_especifico_fd = accept(socket_servidor_fd, (struct sockaddr *)&client_addr,  &client_len);
        //Si hay un fallo se lanza error y se continúa escuchando
        if (socket_especifico_fd < 0){
            perror("accept");
            continue;
        }

        printf("Conexión aceptada con IP %s y puerto %d\n", 
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));

        // Leer el código de operación
        unsigned char codigo_operacion;
        
        if (recvMessage(socket_especifico_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0){
            perror("recvMessage codigo_operacion");
            //Cerrar el descriptor si hay un fallo
            close(socket_especifico_fd);
            continue;
        }

        //Si se llega aquí es que se ha recibido correctamente
        printf("Código de operación recibido: %u\n", codigo_operacion);

        if (codigo_operacion != OP_EXIST) {
            int32_t resultado_error = htonl(-1);

            if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
                perror("sendMessage resultado_error");
            }

            close(socket_especifico_fd);
            continue;
        }

        /* Si llega aquí, la operación es EXIST */
        if (procesar_exist(socket_especifico_fd) < 0) {
            close(socket_especifico_fd);
            continue;
        }

        close(socket_especifico_fd);
    }

    return 0;
}