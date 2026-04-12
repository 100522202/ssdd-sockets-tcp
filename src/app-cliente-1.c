/* Fichero muy sencillo: solo set value y exist*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    // ----------------------------------------------------------------
    // === PRUEBA 6: INTEGRIDAD DE DATOS LARGOS (LÍMITES 255 CHARS) ===
    // ----------------------------------------------------------------
    printf("\n\n=== INICIO PRUEBA 6: LÍMITES DE CADENA (255 CHARS) ===\n");

    char key_larga[256];
    char v1_largo[256];
    
    // Rellenamos con caracteres 'k' y 'v' hasta 255 y añadimos el fin de cadena
    memset(key_larga, 'k', 255);
    key_larga[255] = '\0';
    memset(v1_largo, 'v', 255);
    v1_largo[255] = '\0';

    printf("[CLIENTE]: Probando set_value con clave y valor1 de 255 caracteres...\n");
    res = set_value(key_larga, v1_largo, N, v2, v3);
    
    if (res == 0) {
        printf("[CLIENTE]: OK -> Servidor aceptó los límites máximos.\n");
        
        // Verificamos la integridad con get_value
        char v1_recuperado[256];
        int N_recuperado;
        float v2_recuperado[32];
        struct Paquete v3_recuperado;

        if (get_value(key_larga, v1_recuperado, &N_recuperado, v2_recuperado, &v3_recuperado) == 0) {
            if (strcmp(v1_largo, v1_recuperado) == 0) {
                printf("[CLIENTE]: OK -> Verificación de integridad exitosa (255 bytes íntegros).\n");
            } else {
                printf("[CLIENTE]: ERROR -> Los datos recuperados están corruptos o truncados.\n");
            }
        } else {
            printf("[CLIENTE]: ERROR -> No se pudo recuperar la clave larga recién insertada.\n");
        }
    } else {
        printf("[CLIENTE]: ERROR -> El servidor rechazó el límite de 255 caracteres (retorno %d).\n", res);
    }
    
    printf("=== FIN PRUEBA 6 ===\n\n");

    return 0;
}