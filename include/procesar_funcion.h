#ifndef PROCESAR_H
#define PROCESAR_H
/* Declaración de las funciones auxiliares para procesar
cada función de la API */

// Definir códigos de las operaciones

typedef enum {
    OP_SET = 1,
    OP_GET = 2,
    OP_MODIFY = 3,
    OP_DELETE = 4,
    OP_EXIST = 5,
    OP_DESTROY = 6
} op_t;

// Las propias funciones

int procesar_set(int socket_especifico_fd);
int procesar_get(int socket_especifico_fd);
int procesar_modify(int socket_especifico_fd);
int procesar_delete(int socket_especifico_fd);
int procesar_exist(int socket_especifico_fd);
int procesar_destroy(int socket_especifico_fd);

#endif