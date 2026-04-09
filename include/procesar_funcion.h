#ifndef PROCESAR_H
#define PROCESAR_H
/* Declaración de las funciones auxiliares para procesar
cada función de la API */

// Las propias funciones

int procesar_set(int socket_especifico_fd);
int procesar_get(int socket_especifico_fd);
int procesar_modify(int socket_especifico_fd);
int procesar_delete(int socket_especifico_fd);
int procesar_exist(int socket_especifico_fd);
int procesar_destroy(int socket_especifico_fd);

#endif