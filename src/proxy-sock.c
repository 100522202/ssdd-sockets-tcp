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

// Función interna del proxy para leer IP_TUPLAS y PORT_TUPLAS
static int leer_configuracion_servidor(const char **ip_servidor,  uint16_t *puerto_servidor) {
    // Leer variables de entorno
    const char *ip_tuplas = getenv("IP_TUPLAS");
    const char *port_tuplas = getenv("PORT_TUPLAS");

    if (ip_tuplas == NULL) {
        printf("ERROR: variable de entorno IP_TUPLAS no definida\n");
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

    // Si se llega aquí es que se ha leído correctamente
    printf("Configuración del servidor leída correctamente: IP=%s, puerto=%u\n", ip_servidor, puerto_servidor);

    // 2. Crear el socket del cliente
    int socket_cliente_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_cliente_fd < 0){
        perror("socket cliente");
        return -1;
    }

    printf("Socket TCP del cliente creado correctamente\n");

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

    printf("Dirección del servidor creada correctamente\n");

    // 4. Conectar
    if (connect(socket_cliente_fd, (struct sockaddr *)&direccion_servidor,
                sizeof(direccion_servidor)) < 0) {
        perror("connect cliente");
        close(socket_cliente_fd);
        return -1;
    }

    printf("Conexión entre cliente y servidor establecida correctamente\n");

    // Devolver el socket del cliente ya conectado correctamente
    return socket_cliente_fd;

}

int exist(char *key) {

    // 1. Obtener el socket ya conectado al servidor, se utiliza la función auxiliar
    int socket_cliente_fd = obtener_socket_conectado();

    // Validaciones
    if (socket_cliente_fd < 0){
        perror("socket cliente");
        return -1;
    }

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