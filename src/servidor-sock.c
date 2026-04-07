#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define NUMBER_OF_PORTS 65535

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
    socket_servidor_addr.sin_port = htons(puerto);
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
        return -1;
    }


    return 0;
}