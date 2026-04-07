#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#define NUMBER_OF_PORTS 65535

//Funcionon interna del proxy para leer IP_TUPLAS y PORT_TUPLAS
static int leer_configuración_servidor(const char **ip_servidor,  uint16_t *puerto_servidor) {
    //Leer variables de entorno
    const char *ip_tuplas = getenv("IP_TUPLAS");
    const char *port_tuplas = getenv("PORT_TUPLAS");

    if (ip_tuplas == NULL) {
        perror("ERROR: variable de entorno IP_TUPLAS no definida\n");
        return -1;
    }

    if (port_tuplas == NULL) {
        printf("ERROR: variable de entorno PORT_TUPLAS no definida\n");
        return -1;
    }

    printf("IP_TUPLAS = %s\n", ip_tuplas);
    printf("PORT_TUPLAS = %s\n", port_tuplas);

    //Convertir el puerto de texto a número 
    char *endptr;
    errno = 0;
    // Se usa strtol para mayor robustez
    long puerto = strtol(port_tuplas, &endptr, 10);

    // Validar formato correcto
    if ((errno == ERANGE && (puerto == LONG_MAX || puerto == LONG_MIN)) || (errno != 0 && puerto == 0)) {
        perror("strtol");
        return -1;
    }

    if (endptr == port_tuplas) {
        printf("ERROR: PORT_TUPLAS no es un número\n");
        return -1;
    }

    if (*endptr != '\0') {
        printf("ERROR: PORT_TUPLAS contiene caracteres inválidos\n");
        return -1;
    }

    if (puerto < 0 || puerto > NUMBER_OF_PORTS) {
        printf("ERROR: puerto fuera de rango\n");
        return -1;
    }

    //Devolver resultados
    *ip_servidor = ip_tuplas;
    *puerto_servidor = (uint16_t)puerto;

    return 0;
}