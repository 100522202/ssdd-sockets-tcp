#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "mensajes.h"
#include "procesar-funcion.h"
#include "claves.h"

// TODO: necesitamos otro mutex para proteger la lista enlazada con las tuplas (si lo hacemos aquí es extern o algo así)


/* ---- FUNCIONES QUE SE COMUNICARÁN CON LA API ---- */

/* Estas funciones pertenecientes al lado del servidor
tienen como objetivo realizar la llamada a las funciones
reales de la API. Para ello, harán el unmarshalling de los
parámetros (convertirlos de bytes a su formato original)
y llamarán a las funciones originales. */


int procesar-exist(int socket_especifico_fd) {
    // 1. Leer la longitud de la clave
    int32_t longitud_clave_red;

    if (recvMessage(socket_especifico_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        perror("recvMessage longitud_clave_red");
        return -1;
    }

    // Pasar la longitud de formato de red a formato de máquina
    int32_t longitud_clave = ntohl(longitud_clave_red);

    // Validar la longitud
    if (longitud_clave < 0 || longitud_clave > 255) {
        int32_t resultado_error = htonl(-1);

        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error longitud clave");
            return -1;
        }

        return 0;
    }

    printf("Longitud de clave recibida: %d\n", longitud_clave);

    // 2. Leer la clave
    char clave[256];

    if (recvMessage(socket_especifico_fd, clave, (size_t)longitud_clave) < 0) {
        perror("recvMessage clave");
        return -1;
    }

    // Añadir fin de cadena para usarla como string de C
    clave[longitud_clave] = '\0';

    printf("Clave recibida: %s\n", clave);

    // 3. Llamar a la lógica real
    int resultado_exist = exist(clave);

    printf("Resultado de exist(\"%s\") = %d\n", clave, resultado_exist);

    // 4. Convertir el resultado a formato de red
    int32_t resultado_exist_red = htonl(resultado_exist);

    // 5. Enviar respuesta al cliente
    if (sendMessage(socket_especifico_fd, &resultado_exist_red, sizeof(resultado_exist_red)) < 0) {
        perror("sendMessage resultado_exist_red");
        return -1;
    }

    return 0;
}

int procesar-set(int socket_especifico_fd) {
    // 1. Leer la longitud de la clave
    int32_t longitud_clave_red;

    if (recvMessage(socket_especifico_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        perror("recvMessage longitud_clave_red");
        return -1;
    }

    // Pasar la longitud de formato de red a formato de máquina
    int32_t longitud_clave = ntohl(longitud_clave_red);

    // Validar la longitud
    if (longitud_clave < 0 || longitud_clave > 255) {
        int32_t resultado_error = htonl(-1);

        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error longitud clave");
            return -1;
        }

        return 0;
    }

    printf("Longitud de clave recibida: %d\n", longitud_clave);

    // 2. Leer la clave
    char clave[256];

    if (recvMessage(socket_especifico_fd, clave, (size_t)longitud_clave) < 0) {
        perror("recvMessage clave");
        return -1;
    }

    // Añadir fin de cadena para usarla como string de C
    clave[longitud_clave] = '\0';

    printf("Clave recibida: %s\n", clave);

    // 3. Leer value1 (longitud y cadena)
    int32_t len_v1_red;
    char value1[256];

    // Leer longitud
    if (recvMessage(socket_especifico_fd, &len_v1_red, sizeof(int32_t)) < 0){
        perror("recvMessage len value1");
        return -1;
    }

    // Pasar a formato máquina
    int32_t len_v1 = ntohl(len_v1_red);

    // Validar
    if (len_v1 < 0 || len_v1 > 255){
        int32_t resultado_error_v1 = htonl(-1);
        if (sendMessage(socket_especifico_fd, &resultado_error_v1, sizeof(resultado_error_v1)) < 0) {
            perror("sendMessage resultado_error longitud v1");
            return -1;
        }

        return 0;
    }

    printf("Len value1 recibida: %d\n", len_v1);

    // Leer la cadena
    if (recvMessage(socket_especifico_fd, value1, len_v1) < 0){
        perror("recvMessage value1");
        return -1;
    }

    // Añadir fin de cadena para usarla como string de C
    value1[len_v1] = '\0';

    printf("Value1 recibida: %s\n", value1);

    // 4. Leer N_value2
    int32_t N_value2_red;

    if (recvMessage(socket_especifico_fd, &N_value2_red, sizeof(N_value2)) < 0) {
        perror("recvMessage N_value2");
        return -1;
    }

    // Pasar N_value2 de formato de red a formato de máquina
    int32_t N_value2 = ntohl(N_value2_red);

    // Validar N_value2
    if (N_value2 < 1 || N_value2 > 32) {
        int32_t resultado_error = htonl(-1);
        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error longitud");
            return -1;
        }

        return 0;
    }

    printf("N_value2 recibido: %d\n", N_value2);

    // 5. Leer V_value2
    float V_value2[N_value2];

    if (recvMessage(socket_especifico_fd, V_value2, N_value2 * sizeof(float)) < 0) {
        perror("recvMessage V_Value2");
        return -1;
    }

    // 6. Leer value3
    struct Paquete value3;
    int32_t x_red, y_red, z_red;

    if (recvMessage(socket_especifico_fd, &x_red, sizeof(int32_t)) < 0){
        perror("recvMessage value3 x");
        return -1;
    }
    if (recvMessage(socket_especifico_fd, &y_red, sizeof(int32_t)) < 0){
        perror("recvMessage value3 y");
        return -1;
    }
    if (recvMessage(socket_especifico_fd, &z_red, sizeof(int32_t)) < 0){
        perror("recvMessage value3 z");
        return -1;
    }

    // Pasar a formato máquina y almacenar en el struct
    value3.x = ntohl(x_red);
    value3.y = ntohl(y_red);
    value3.z = ntohl(z_red);

    // 7. Llamar a la lógica real
    // TODO: supuestamente aquí va el mutex
    /*
    pthread_mutex_lock(&mutex_lista);
    int resultado = set_value(key, value1, n_v2, v_value2, v3);
    pthread_mutex_unlock(&mutex_lista);
    */

    int resultado_set_value = set_value(clave, value1, N_value2, V_value2, value3);

    printf("Resultado de exist(\"%s\") = %d\n", clave, resultado_set_value);

    // 8. Convertir el resultado a formato de red
    int32_t resultado_set_value_red = htonl(resultado_set_value);

    // 9. Enviar respuesta al cliente
    if (sendMessage(socket_especifico_fd, &resultado_set_value_red, sizeof(resultado_set_value_red)) < 0) {
        perror("sendMessage resultado_set_value_red");
        return -1;
    }

    return 0;
}