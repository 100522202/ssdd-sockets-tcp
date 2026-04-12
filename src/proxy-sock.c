#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "claves.h"
#include "mensajes.h"

#define NUMBER_OF_PORTS 65535

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

// Función interna del proxy para leer IP_TUPLAS y PORT_TUPLAS
static int leer_configuracion_servidor(const char **ip_servidor,  uint16_t *puerto_servidor) {
    // Leer variables de entorno
    const char *ip_tuplas = getenv("IP_TUPLAS");
    const char *port_tuplas = getenv("PORT_TUPLAS");

    if (ip_tuplas == NULL) {
        fprintf(stderr, "ERROR: variable de entorno IP_TUPLAS no definida\n");
        return -1;
    }

    if (port_tuplas == NULL) {
        fprintf(stderr, "ERROR: variable de entorno PORT_TUPLAS no definida\n");
        return -1;
    }

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

    // Devolver resultados
    *ip_servidor = ip_tuplas;
    *puerto_servidor = (uint16_t)puerto;

    return 0;
}

// Función interna del proxy: obtiene un socket TCP ya conectado al servidor
static int obtener_socket_conectado(void) {
    // 1. Leer la configuración del servidor
    const char *ip_servidor;
    uint16_t puerto_servidor;

    // Se pasan por referencia
    if (leer_configuracion_servidor(&ip_servidor, &puerto_servidor) < 0) {
        return -1;
    }

    // 2. Crear el socket del cliente
    int socket_cliente_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_cliente_fd < 0){
        perror("socket cliente");
        return -1;
    }

    // 3. Crear la dirección del servidor
    struct sockaddr_in direccion_servidor;

    // Se inicializa a 0
    memset(&direccion_servidor, 0, sizeof(direccion_servidor));

    // Rellenar los atributos de la dirección del servidor
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(puerto_servidor);

    // Verificar el nombre o ip del servidor
    struct hostent *informacion_host = gethostbyname(ip_servidor);
    if (informacion_host == NULL) {
        printf("ERROR: no se pudo resolver el valor de IP_TUPLAS\n");
        close(socket_cliente_fd);
        return -1;
    }
    
    // Ya se tiene la ip, ahora se copia dentro de sin_addr
    memcpy(&direccion_servidor.sin_addr, informacion_host->h_addr, informacion_host->h_length);

    // 4. Conectar
    if (connect(socket_cliente_fd, (struct sockaddr *)&direccion_servidor,
                sizeof(direccion_servidor)) < 0) {
        perror("connect cliente");
        close(socket_cliente_fd);
        return -1;
    }

    // Devolver el socket del cliente ya conectado correctamente
    return socket_cliente_fd;

}

/* ---- FUNCIONES DE LA API EN EL PROXY ---- */

/* Estas funciones tienen como objetivo suplantar a la 
API real de cara al cliente, para ello harán el 
marshalling de los parámetros (pasarlos a bytes) y se 
los enviarán al socket conectado a servidor, que llamará
realmente a las funciones de la API */

// Flujo de datos (con tamaño en B): 
// [Cód_OP (1)][Len_clave(4)][Clave(Len_clave)][Len_value1(4)][value1(Len_value1)][N_value2(4)][V_value2(N_value2 * 4)][value3(12)]
// para memoria: len clave basta con 1 B pero si lo hacemos con 4 tenemos escalabilidad porque somos unos cracks 
int exist(char *key) {

    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validar fd
    if (socket_cliente_fd < 0){
        return -1;
    }

    // Validar punteros
    if (key == NULL) {
        close(socket_cliente_fd);
        return -1;
    }

    // 2. Obtener y validar la longitud de la clave
    int32_t longitud_clave = (int32_t)strlen(key);
    if (longitud_clave > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 3. Enviar el código de operación
    unsigned char codigo_operacion = OP_EXIST;
    if (sendMessage(socket_cliente_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 4. Enviar la longitud de la clave
    int32_t longitud_clave_red = htonl(longitud_clave);
    if (sendMessage(socket_cliente_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 5. Enviar la clave
    if (sendMessage(socket_cliente_fd, key, (size_t)longitud_clave) < 0) {
        close(socket_cliente_fd);
        return -1;
    }


    // 6. Recibir el resultado y convertirlo a formato máquina
    int32_t resultado_red;
    if (recvMessage(socket_cliente_fd, &resultado_red, sizeof(resultado_red)) < 0) {
        close(socket_cliente_fd);
        return -1;       
    }

    int resultado = ntohl(resultado_red);

    // 7. Cerrar el socket y devolver el resultado
    close(socket_cliente_fd);

    return resultado;
    
}


int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {

    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validar fd
    if (socket_cliente_fd < 0){
        return -1;
    }

    // Validar punteros
    if (key == NULL || value1 == NULL || V_value2 == NULL) {
        close(socket_cliente_fd);
        return -1;
    }

    // 2. Obtener y validar la longitud de la clave
    int32_t longitud_clave = (int32_t)strlen(key);
    if (longitud_clave > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 3. Obtener y validar la longitud de value1
    int32_t longitud_value1 = (int32_t)strlen(value1);
    if (longitud_value1 > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 4. Validar N_value2 y cambiar su formato
    if (N_value2 > 32 || N_value2 < 1){
        close(socket_cliente_fd);
        return -1;
    }

    int32_t new_N_value2 = (int32_t)N_value2;

    // 5. Enviar el código de operación
    unsigned char codigo_operacion = OP_SET;
    if (sendMessage(socket_cliente_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 6. Enviar la longitud de la clave

    int32_t longitud_clave_red = htonl(longitud_clave);
    if (sendMessage(socket_cliente_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 7. Enviar la clave
    if (sendMessage(socket_cliente_fd, key, (size_t)longitud_clave) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 8. Enviar longitud value1
    int32_t longitud_value1_red = htonl(longitud_value1);
    if (sendMessage(socket_cliente_fd, &longitud_value1_red, sizeof(longitud_value1_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 9. Enviar cadena value1
    if (sendMessage(socket_cliente_fd, value1, (size_t)longitud_value1) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 10. Enviar N_value2
    int32_t Nvalue2_red = htonl(new_N_value2);
    if (sendMessage(socket_cliente_fd, &Nvalue2_red, sizeof(Nvalue2_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 11. Enviar V_value2
    for (int i = 0; i < N_value2; i++) {
        uint32_t valor_red = float_to_network(V_value2[i]);
        if (sendMessage(socket_cliente_fd, &valor_red, sizeof(valor_red)) < 0) {
            close(socket_cliente_fd);
            return -1;
        }
    }

    // 12. Enviar el struct paquete value3

    // "Marshallizar" cada uno de sus enteros
    int32_t x_net = htonl(value3.x);
    int32_t y_net = htonl(value3.y);
    int32_t z_net = htonl(value3.z);

    // Enviarlos

    if (sendMessage(socket_cliente_fd, &x_net, sizeof(int32_t)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    if (sendMessage(socket_cliente_fd, &y_net, sizeof(int32_t)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    if (sendMessage(socket_cliente_fd, &z_net, sizeof(int32_t)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 13. Recibir el resultado y convertirlo a formato máquina
    int32_t resultado_red;
    if (recvMessage(socket_cliente_fd, &resultado_red, sizeof(resultado_red)) < 0) {
        close(socket_cliente_fd);
        return -1;       
    }

    int resultado = ntohl(resultado_red);

    // 14. Cerrar el socket y devolver el resultado
    close(socket_cliente_fd);

    return resultado;
    
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validar fd
    if (socket_cliente_fd < 0){
        return -1;
    }

    // Validar punteros
    if (key == NULL || value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL) {
        close(socket_cliente_fd);
        return -1;
    }

    // 2. Obtener y validar la longitud de la clave
    int32_t longitud_clave = (int32_t)strlen(key);
    if (longitud_clave > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 3. Enviar el código de operación
    unsigned char codigo_operacion = OP_GET;
    if (sendMessage(socket_cliente_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 4. Enviar la longitud de la clave
    int32_t longitud_clave_red = htonl(longitud_clave);
    if (sendMessage(socket_cliente_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 5. Enviar la clave
    if (sendMessage(socket_cliente_fd, key, (size_t)longitud_clave) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 6. Recibir el resultado y convertirlo a formato máquina
    int32_t resultado_red;
    if (recvMessage(socket_cliente_fd, &resultado_red, sizeof(resultado_red)) < 0) {
        close(socket_cliente_fd);
        return -1;       
    }

    int resultado = ntohl(resultado_red);

    // Si el resultado es éxito, recibir el resto de datos
    if (resultado == 0) {
        // Recibir longitud value1
        int32_t len_v1_red;
        if (recvMessage(socket_cliente_fd, &len_v1_red, sizeof(int32_t)) < 0) {
            close(socket_cliente_fd); return -1;
        }
        int32_t len_v1 = ntohl(len_v1_red);
        if (len_v1 < 0 || len_v1 > 255) {
            close(socket_cliente_fd);
            return -1;
        }

        // Recibir cadena value1
        if (recvMessage(socket_cliente_fd, value1, (size_t)len_v1) < 0) {
            close(socket_cliente_fd); return -1;
        }
        value1[len_v1] = '\0';

        // Recibir N_value2
        int32_t n2_red;
        if (recvMessage(socket_cliente_fd, &n2_red, sizeof(int32_t)) < 0) {
            close(socket_cliente_fd); return -1;
        }
        *N_value2 = ntohl(n2_red);
        if (*N_value2 < 1 || *N_value2 > 32) {
            close(socket_cliente_fd);
            return -1;
        }

        // Recibir V_value2
        for (int i = 0; i < *N_value2; i++) {
            uint32_t valor_red;
            if (recvMessage(socket_cliente_fd, &valor_red, sizeof(valor_red)) < 0) {
                close(socket_cliente_fd);
                return -1;
            }
            V_value2[i] = float_from_network(valor_red);
        }

        // Recibir el struct paquete value3
        int32_t x_net, y_net, z_net;
        if (recvMessage(socket_cliente_fd, &x_net, sizeof(int32_t)) < 0) {
            close(socket_cliente_fd); return -1;
        }
        if (recvMessage(socket_cliente_fd, &y_net, sizeof(int32_t)) < 0) {
            close(socket_cliente_fd); return -1;
        }
        if (recvMessage(socket_cliente_fd, &z_net, sizeof(int32_t)) < 0) {
            close(socket_cliente_fd); return -1;
        }
        value3->x = ntohl(x_net);
        value3->y = ntohl(y_net);
        value3->z = ntohl(z_net);
    }

    // 7. Cerrar el socket y devolver el resultado
    close(socket_cliente_fd);
    return resultado;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validar fd
    if (socket_cliente_fd < 0){
        return -1;
    }

    // Validar punteros
    if (key == NULL || value1 == NULL || V_value2 == NULL) {
        close(socket_cliente_fd);
        return -1;
    }

    // 2. Obtener y validar la longitud de la clave
    int32_t longitud_clave = (int32_t)strlen(key);
    if (longitud_clave > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 3. Obtener y validar la longitud de value1
    int32_t longitud_value1 = (int32_t)strlen(value1);
    if (longitud_value1 > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 4. Validar N_value2 y cambiar su formato
    if (N_value2 > 32 || N_value2 < 1){
        close(socket_cliente_fd);
        return -1;
    }

    int32_t new_N_value2 = (int32_t)N_value2;

    // 5. Enviar el código de operación
    unsigned char codigo_operacion = OP_MODIFY;
    if (sendMessage(socket_cliente_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 6. Enviar la longitud de la clave
    int32_t longitud_clave_red = htonl(longitud_clave);
    if (sendMessage(socket_cliente_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 7. Enviar la clave
    if (sendMessage(socket_cliente_fd, key, (size_t)longitud_clave) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 8. Enviar longitud value1
    int32_t longitud_value1_red = htonl(longitud_value1);
    if (sendMessage(socket_cliente_fd, &longitud_value1_red, sizeof(longitud_value1_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 9. Enviar cadena value1
    if (sendMessage(socket_cliente_fd, value1, (size_t)longitud_value1) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 10. Enviar N_value2
    int32_t Nvalue2_red = htonl(new_N_value2);
    if (sendMessage(socket_cliente_fd, &Nvalue2_red, sizeof(Nvalue2_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 11. Enviar V_value2
    for (int i = 0; i < N_value2; i++) {
        uint32_t valor_red = float_to_network(V_value2[i]);
        if (sendMessage(socket_cliente_fd, &valor_red, sizeof(valor_red)) < 0) {
            close(socket_cliente_fd);
            return -1;
        }
    }

    // 12. Enviar el struct paquete value3

    // "Marshallizar" cada uno de sus enteros
    int32_t x_net = htonl(value3.x);
    int32_t y_net = htonl(value3.y);
    int32_t z_net = htonl(value3.z);

    // Enviarlos
    if (sendMessage(socket_cliente_fd, &x_net, sizeof(int32_t)) < 0) {
        close(socket_cliente_fd); return -1;
    }
    if (sendMessage(socket_cliente_fd, &y_net, sizeof(int32_t)) < 0) {
        close(socket_cliente_fd); return -1;
    }
    if (sendMessage(socket_cliente_fd, &z_net, sizeof(int32_t)) < 0) {
        close(socket_cliente_fd); return -1;
    }

    // 13. Recibir el resultado y convertirlo a formato máquina
    int32_t resultado_red;
    if (recvMessage(socket_cliente_fd, &resultado_red, sizeof(resultado_red)) < 0) {
        close(socket_cliente_fd);
        return -1;       
    }

    int resultado = ntohl(resultado_red);

    // 14. Cerrar el socket y devolver el resultado
    close(socket_cliente_fd);
    return resultado;
}

int delete_key(char *key) {
    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validar fd
    if (socket_cliente_fd < 0){
        return -1;
    }

    // Validar punteros
    if (key == NULL) {
        close(socket_cliente_fd);
        return -1;
    }

    // 2. Obtener y validar la longitud de la clave
    int32_t longitud_clave = (int32_t)strlen(key);
    if (longitud_clave > 255) {
        close(socket_cliente_fd);
        return -1;
    }

    // 3. Enviar el código de operación
    unsigned char codigo_operacion = OP_DELETE;
    if (sendMessage(socket_cliente_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 4. Enviar la longitud de la clave
    int32_t longitud_clave_red = htonl(longitud_clave);
    if (sendMessage(socket_cliente_fd, &longitud_clave_red, sizeof(longitud_clave_red)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 5. Enviar la clave
    if (sendMessage(socket_cliente_fd, key, (size_t)longitud_clave) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 6. Recibir el resultado y convertirlo a formato máquina
    int32_t resultado_red;
    if (recvMessage(socket_cliente_fd, &resultado_red, sizeof(resultado_red)) < 0) {
        close(socket_cliente_fd);
        return -1;       
    }

    int resultado = ntohl(resultado_red);

    // 7. Cerrar el socket y devolver el resultado
    close(socket_cliente_fd);
    return resultado;
}

int destroy() {
    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validar fd
    if (socket_cliente_fd < 0){
        return -1;
    }

    // 2. Enviar el código de operación
    unsigned char codigo_operacion = OP_DESTROY;
    if (sendMessage(socket_cliente_fd, &codigo_operacion, sizeof(codigo_operacion)) < 0) {
        close(socket_cliente_fd);
        return -1;
    }

    // 3. Recibir el resultado y convertirlo a formato máquina
    int32_t resultado_red;
    if (recvMessage(socket_cliente_fd, &resultado_red, sizeof(resultado_red)) < 0) {
        close(socket_cliente_fd);
        return -1;       
    }

    int resultado = ntohl(resultado_red);

    // 4. Cerrar el socket y devolver el resultado
    close(socket_cliente_fd);
    return resultado;
}
