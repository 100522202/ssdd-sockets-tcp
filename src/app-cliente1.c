/* Fichero muy sencillo: solo set value y exist*/
#include <stdio.h>
#include <stdlib.h>
#include "claves.h"

int main() {
    int res;
    
    // 1. Datos de prueba para la tupla
    char *key = "clave_test";
    char *v1 = "Mensaje de prueba";
    int N = 3;
    float v2[] = {1.5f, 2.5f, 3.5f};
    struct Paquete v3 = {100, 200, 300};

    printf("=== CLIENTE DE PRUEBAS INICIALES (SET y EXIST) ===\n\n");

    // 2. Probar set_value
    printf("[CLIENTE]: Intentando insertar clave: '%s'...\n", key);
    res = set_value(key, v1, N, v2, v3);
    
    if (res == 0) {
        printf("[CLIENTE]: OK -> set_value ejecutado con éxito.\n");
    } else {
        printf("[CLIENTE]: ERROR -> set_value falló (retornó %d).\n", res);
    }

    printf("\n------------------------------------------------\n\n");

    // 3. Probar exist con la clave recién creada
    printf("[CLIENTE]: Verificando si '%s' existe...\n", key);
    res = exist(key);
    
    if (res == 1) {
        printf("[CLIENTE]: OK -> La clave existe en el servidor.\n");
    } else if (res == 0) {
        printf("[CLIENTE]: ERROR -> El servidor dice que la clave NO existe.\n");
    } else {
        printf("[CLIENTE]: ERROR -> Fallo en la comunicación con el servidor.\n");
    }

    // 4. Probar exist con una clave que no pusimos
    printf("\n[CLIENTE]: Verificando clave inexistente ('fantasma')...\n");
    res = exist("fantasma");
    if (res == 0) {
        printf("[CLIENTE]: OK -> El servidor confirma correctamente que no existe.\n");
    } else {
        printf("[CLIENTE]: ERROR -> Comportamiento inesperado (retornó %d).\n", res);
    }

    return 0;
}