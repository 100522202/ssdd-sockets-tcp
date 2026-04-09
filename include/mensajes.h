#ifndef MENSAJES_H
#define MENSAJES_H

#include <stddef.h>

// Definir códigos de las operaciones

typedef enum {
    OP_SET = 1,
    OP_GET = 2,
    OP_MODIFY = 3,
    OP_DELETE = 4,
    OP_EXIST = 5,
    OP_DESTROY = 6
} op_t;

/* Se incluyen las dos funciones auxiliares relacionadas 
con el envío/recepción de mensajes */

// Prototipos de las funciones de red
int sendMessage(int socket_fd, const void *buffer, size_t num_bytes);
int recvMessage(int socket_fd, void *buffer_destino, size_t num_bytes);

#endif
