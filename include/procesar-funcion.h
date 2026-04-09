#ifndef procesar-H
#define procesar-H
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

int procesar-set(int socket_especifico_fd);
int procesar-get(int socket_especifico_fd);
int procesar-modify(int socket_especifico_fd);
int procesar-delete(int socket_especifico_fd);
int procesar-exist(int socket_especifico_fd);
int procesar-destroy(int socket_especifico_fd);

#endif