#ifndef MENSAJES_H
#define MENSAJES_H

#include <stddef.h>

/* Se incluyen las dos funciones auxiliares relacionadas 
con el envío/recepción de mensajes */

// Prototipos de las funciones de red
int sendMessage(int socket_fd, const void *buffer, size_t num_bytes);
int recvMessage(int socket_fd, void *buffer_destino, size_t num_bytes);

#endif
