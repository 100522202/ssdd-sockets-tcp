#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "mensajes.h"
#include "procesar_funcion.h"
#include "claves.h"

// TODO: necesitamos otro mutex para proteger la lista enlazada con las tuplas (si lo hacemos aquí es extern o algo así)

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