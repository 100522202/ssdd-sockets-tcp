#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#include "mensajes.h"
#include "procesar_funcion.h"
#include "claves.h"

static pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;

static uint32_t float_to_network(float valor) {
    uint32_t valor_u32;
    memcpy(&valor_u32, &valor, sizeof(valor_u32));
    return htonl(valor_u32);
}

static float float_from_network(uint32_t valor_red) {
    uint32_t valor_host = ntohl(valor_red);
    float valor;
    memcpy(&valor, &valor_host, sizeof(valor));
    return valor;
}


/* ---- FUNCIONES QUE SE COMUNICARÁN CON LA API ---- */

/* Estas funciones pertenecientes al lado del servidor
tienen como objetivo realizar la llamada a las funciones
reales de la API. Para ello, harán el unmarshalling de los
parámetros (convertirlos de bytes a su formato original)
y llamarán a las funciones originales. */


int procesar_exist(int socket_especifico_fd) {
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

    // 2. Leer la clave
    char clave[256];
    if (recvMessage(socket_especifico_fd, clave, (size_t)longitud_clave) < 0) {
        perror("recvMessage clave");
        return -1;
    }
    clave[longitud_clave] = '\0';

    // 3. Llamar a la lógica real
    pthread_mutex_lock(&mutex_lista);
    int resultado_exist = exist(clave);
    pthread_mutex_unlock(&mutex_lista);

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

int procesar_set(int socket_especifico_fd) {
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

    if (recvMessage(socket_especifico_fd, &N_value2_red, sizeof(N_value2_red)) < 0) {
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
    for (int i = 0; i < N_value2; i++) {
        uint32_t valor_red;
        if (recvMessage(socket_especifico_fd, &valor_red, sizeof(valor_red)) < 0) {
            perror("recvMessage V_Value2");
            return -1;
        }
        V_value2[i] = float_from_network(valor_red);
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
    pthread_mutex_lock(&mutex_lista);
    int resultado_set_value = set_value(clave, value1, N_value2, V_value2, value3);
    pthread_mutex_unlock(&mutex_lista);

    printf("Resultado de set_value(\"%s\") = %d\n", clave, resultado_set_value);

    // 8. Convertir el resultado a formato de red
    int32_t resultado_set_value_red = htonl(resultado_set_value);

    // 9. Enviar respuesta al cliente
    if (sendMessage(socket_especifico_fd, &resultado_set_value_red, sizeof(resultado_set_value_red)) < 0) {
        perror("sendMessage resultado_set_value_red");
        return -1;
    }

    return 0;
}

int procesar_get(int socket_especifico_fd) {
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
    clave[longitud_clave] = '\0';

    printf("Clave recibida: %s\n", clave);

    // 3. Preparar variables para recibir los datos de la lógica real
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;

    // 4. Llamar a la lógica real
    pthread_mutex_lock(&mutex_lista);
    int resultado_get_value = get_value(clave, value1, &N_value2, V_value2, &value3);
    pthread_mutex_unlock(&mutex_lista);

    printf("Resultado de get_value(\"%s\") = %d\n", clave, resultado_get_value);

    // 5. Convertir el resultado a formato de red y enviar
    int32_t resultado_get_value_red = htonl(resultado_get_value);
    if (sendMessage(socket_especifico_fd, &resultado_get_value_red, sizeof(resultado_get_value_red)) < 0) {
        perror("sendMessage resultado_get_value_red");
        return -1;
    }

    // 6. Si la clave existe (resultado 0), enviar el resto de la tupla
    if (resultado_get_value == 0) {
        // Enviar longitud value1
        int32_t len_v1 = (int32_t)strlen(value1);
        int32_t len_v1_red = htonl(len_v1);
        if (sendMessage(socket_especifico_fd, &len_v1_red, sizeof(int32_t)) < 0) {
            perror("sendMessage len_v1_red");
            return -1;
        }

        // Enviar cadena value1
        if (sendMessage(socket_especifico_fd, value1, (size_t)len_v1) < 0) {
            perror("sendMessage value1");
            return -1;
        }

        // Enviar N_value2
        int32_t n2_red = htonl((int32_t)N_value2);
        if (sendMessage(socket_especifico_fd, &n2_red, sizeof(int32_t)) < 0) {
            perror("sendMessage n2_red");
            return -1;
        }

        // Enviar V_value2
        for (int i = 0; i < N_value2; i++) {
            uint32_t valor_red = float_to_network(V_value2[i]);
            if (sendMessage(socket_especifico_fd, &valor_red, sizeof(valor_red)) < 0) {
                perror("sendMessage V_value2");
                return -1;
            }
        }

        // Enviar value3 (x, y, z) marshallizados
        int32_t x_red = htonl(value3.x);
        int32_t y_red = htonl(value3.y);
        int32_t z_red = htonl(value3.z);

        if (sendMessage(socket_especifico_fd, &x_red, sizeof(int32_t)) < 0) {
            perror("sendMessage value3 x");
            return -1;
        }
        if (sendMessage(socket_especifico_fd, &y_red, sizeof(int32_t)) < 0) {
            perror("sendMessage value3 y");
            return -1;
        }
        if (sendMessage(socket_especifico_fd, &z_red, sizeof(int32_t)) < 0) {
            perror("sendMessage value3 z");
            return -1;
        }
    }

    return 0;
}

int procesar_modify(int socket_especifico_fd) {
    // 1. Leer la longitud de la clave
    int32_t longitud_clave_red;
    if (recvMessage(socket_especifico_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        perror("recvMessage longitud_clave_red");
        return -1;
    }
    int32_t longitud_clave = ntohl(longitud_clave_red);
    if (longitud_clave < 0 || longitud_clave > 255) {
        int32_t resultado_error = htonl(-1);
        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error longitud clave");
            return -1;
        }
        return 0;
    }

    // 2. Leer la clave
    char clave[256];
    if (recvMessage(socket_especifico_fd, clave, (size_t)longitud_clave) < 0) {
        perror("recvMessage clave");
        return -1;
    }
    clave[longitud_clave] = '\0';

    // 3. Leer value1 (longitud y cadena)
    int32_t len_v1_red;
    char value1[256];
    if (recvMessage(socket_especifico_fd, &len_v1_red, sizeof(int32_t)) < 0) {
        perror("recvMessage len value1");
        return -1;
    }
    int32_t len_v1 = ntohl(len_v1_red);
    if (len_v1 < 0 || len_v1 > 255) {
        int32_t resultado_error = htonl(-1);
        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error len value1");
            return -1;
        }
        return 0;
    }
    if (recvMessage(socket_especifico_fd, value1, (size_t)len_v1) < 0) {
        perror("recvMessage value1");
        return -1;
    }
    value1[len_v1] = '\0';

    // 4. Leer N_value2
    int32_t N_value2_red;
    if (recvMessage(socket_especifico_fd, &N_value2_red, sizeof(int32_t)) < 0) {
        perror("recvMessage N_value2");
        return -1;
    }
    int32_t N_value2 = ntohl(N_value2_red);
    if (N_value2 < 1 || N_value2 > 32) {
        int32_t resultado_error = htonl(-1);
        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error N_value2");
            return -1;
        }
        return 0;
    }

    // 5. Leer V_value2
    float V_value2[32];
    for (int i = 0; i < N_value2; i++) {
        uint32_t valor_red;
        if (recvMessage(socket_especifico_fd, &valor_red, sizeof(valor_red)) < 0) {
            perror("recvMessage V_value2");
            return -1;
        }
        V_value2[i] = float_from_network(valor_red);
    }

    // 6. Leer value3
    struct Paquete value3;
    int32_t x_red, y_red, z_red;
    if (recvMessage(socket_especifico_fd, &x_red, sizeof(int32_t)) < 0) return -1;
    if (recvMessage(socket_especifico_fd, &y_red, sizeof(int32_t)) < 0) return -1;
    if (recvMessage(socket_especifico_fd, &z_red, sizeof(int32_t)) < 0) return -1;
    value3.x = ntohl(x_red);
    value3.y = ntohl(y_red);
    value3.z = ntohl(z_red);

    // 7. Llamar a la lógica real
    pthread_mutex_lock(&mutex_lista);
    int resultado_modify = modify_value(clave, value1, N_value2, V_value2, value3);
    pthread_mutex_unlock(&mutex_lista);

    // 8. Enviar respuesta
    int32_t resultado_red = htonl(resultado_modify);
    if (sendMessage(socket_especifico_fd, &resultado_red, sizeof(int32_t)) < 0) {
        perror("sendMessage resultado_modify");
        return -1;
    }

    return 0;
}

int procesar_delete(int socket_especifico_fd) {
    // 1. Leer la longitud de la clave
    int32_t longitud_clave_red;
    if (recvMessage(socket_especifico_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        perror("recvMessage longitud_clave_red");
        return -1;
    }
    int32_t longitud_clave = ntohl(longitud_clave_red);
    if (longitud_clave < 0 || longitud_clave > 255) {
        int32_t resultado_error = htonl(-1);
        if (sendMessage(socket_especifico_fd, &resultado_error, sizeof(resultado_error)) < 0) {
            perror("sendMessage resultado_error longitud clave");
            return -1;
        }
        return 0;
    }

    // 2. Leer la clave
    char clave[256];
    if (recvMessage(socket_especifico_fd, clave, (size_t)longitud_clave) < 0) {
        perror("recvMessage clave");
        return -1;
    }
    clave[longitud_clave] = '\0';

    // 3. Llamar a la lógica real
    pthread_mutex_lock(&mutex_lista);
    int resultado_delete = delete_key(clave);
    pthread_mutex_unlock(&mutex_lista);

    // 4. Enviar respuesta
    int32_t resultado_red = htonl(resultado_delete);
    if (sendMessage(socket_especifico_fd, &resultado_red, sizeof(int32_t)) < 0) {
        perror("sendMessage resultado_delete");
        return -1;
    }

    return 0;
}

int procesar_destroy(int socket_especifico_fd) {
    // 1. Llamar a la lógica real
    pthread_mutex_lock(&mutex_lista);
    int resultado_destroy = destroy();
    pthread_mutex_unlock(&mutex_lista);

    // 2. Enviar respuesta
    int32_t resultado_red = htonl(resultado_destroy);
    if (sendMessage(socket_especifico_fd, &resultado_red, sizeof(int32_t)) < 0) {
        perror("sendMessage resultado_destroy");
        return -1;
    }

    return 0;
}
