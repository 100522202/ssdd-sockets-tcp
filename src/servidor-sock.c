#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define NUMBER_OF_PORTS 65535

int main(int argc, char * argv[]){
    
    if (argc != 2){
        printf("Error en el número de argumentos pasados a servidor\n");
        return -1;
    }
    // Capturar el número de puerto, strtol para robustez

    char * endptr;
    long puerto = strtol(argv[1], &endptr, 10);
    errno = 0;


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

    return 0;
}