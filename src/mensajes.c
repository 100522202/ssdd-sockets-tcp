#include "mensajes.h"
#include <unistd.h>
#include <errno.h>

//Envia num_bytes bytes
int sendMessage(int socket_fd, const void *buffer, size_t num_bytes) {
    //se inicializa a 0 el contador
    size_t enviados = 0;
    const char *puntero_datos = (const char *)buffer;

    while (enviados < num_bytes) {
        ssize_t enviados_ahora = write(
            socket_fd, 
            puntero_datos + enviados,
            num_bytes - enviados
        );

        if (enviados_ahora < 0) {
            //Error al enviar
            return -1; 
        }

        if (enviados_ahora == 0){
            //No se envió nada
            return -1;
        }

        // Si se llega aquí es que se envió correctamente
        enviados += (size_t)enviados_ahora;
    }

    //Se enviaron todos los bytes
    return 0;
}

//Recibe exactamente num_bytes bytes
int recvMessage(int socket_fd, void *buffer_destino, size_t num_bytes) {
    size_t recibidos = 0;
    char *puntero_buffer = (char *)buffer_destino;

    while(recibidos < num_bytes) {
        ssize_t recibidos_ahora = read(
            socket_fd, 
            puntero_buffer + recibidos,
            num_bytes - recibidos
        );

        if (recibidos_ahora < 0) {
            // error al recibir
            return -1;   
        }

        if (recibidos_ahora == 0) {
            // el otro extremo cerró la conexión antes de tiempo
            return -1;   
        }

        // Si se llega aquí es que se recibió correctamente
        recibidos += (size_t)recibidos_ahora;

    }

    //Se recibieron todos los bytes
    return 0;
}