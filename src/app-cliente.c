#include <stdio.h>
#include "claves.h"
#include <string.h>

/*Creamos una función para que nos ayude a la impresion*/
void imprimir(char *key, char *v1, int N, float *v2, struct Paquete *v3){
    printf("Key: %s\n", key);
    printf("  value1: \"%s\"\n", v1);
    printf("  N: %d\n", N);
    printf("  v2: [");
    for (int i = 0; i < N; i++) {
        printf("%s%.2f", (i == 0 ? "" : ", "), v2[i]);
    }
    printf("]\n");
    printf("  v3: {x=%d, y=%d, z=%d}\n", v3->x, v3->y, v3->z);
}

int main ()
{
    /*Creamos el primer ejemplo*/
    char *key = "clave 1";
    char *v1 = "ejemplo de valor 1";
    float v2[] = {2.3, 0.5, 23.45};
    struct Paquete v3;
    v3.x = 10;
    v3.y = 5;
    v3.z = 3;

    /*Creamos el segundo ejemplo*/
    char *key_2 = "clave 2";
    char *v1_2 = "ejemplo de valor 2";
    float v2_2[] = {5.3, 0.9, 233.45, 7.6};
    struct Paquete v3_2;
    v3_2.x = 9;
    v3_2.y = 4;
    v3_2.z = 3;

    printf("Imprimimos nuestras tuplas de prueba:\n\n");
    imprimir(key, v1, 3, v2, &v3);
    printf("\n");
    imprimir(key_2, v1_2, 4, v2_2, &v3_2);

    /* Probamos las funciones */

    printf("\nCasos de éxito de set_value\n");

    int err = set_value(key, v1, 3, v2, v3);
    if (err == -1) {
        printf("Error al insertar la tupla\n");
    } else {
        printf("\nSet_value tuvo éxito\n");
    }

    int err_2 = set_value(key_2, v1_2, 4, v2_2, v3_2);
    if (err_2 == -1) {
        printf("Error al insertar la tupla\n");
    } else {
        printf("\nSet_value tuvo éxito\n\n");
    }

    

    /*Caso de ERROR: probamos a insertar un nuevo valor con una clave que ya se está utilizando*/
    printf("Ahora veremos casos de prueba de error de set_value:\n");
    err = set_value(key, v1, 3, v2, v3);
    if (err == -1){
        printf("    ERROR: clave previamente guardada\n");
    }

    /* Error de inserción --> uno de los parámetros no es correcto*/
    int err_3 = set_value(key_2, v1_2, 99, v2_2, v3_2);
    if (err_3 == -1) {
        printf("    Error al insertar la tupla\n\n");
    }

    /* Probamos la funcion get_value */
    char v1_prueba[256];
    int N_prueba = 0;
    float v2_prueba[32];
    struct Paquete v3_prueba;
    printf("Probamos a continuación la función get_value:\n\n");
    int err_get = get_value(key, v1_prueba, &N_prueba, v2_prueba, &v3_prueba);
    if (err_get == 0) {
        printf("Éxito haciendo get_value, imprimimos tupla de ejemplo:\n");
        imprimir(key, v1_prueba, N_prueba, v2_prueba, &v3_prueba);
        printf("\n");
    } else {
        printf("\nGET ERROR: no existe la clave o falló\n");
    }


    printf("Caso de error de get_value: no existe el elemento:\n");

    int err_get2 = get_value("clave_inexistente", v1_prueba, &N_prueba, v2_prueba, &v3_prueba);
    if (err_get2 == 0) {
        printf("Éxito haciendo get_value, imprimimos tupla de ejemplo:\n");
        imprimir(key, v1_prueba, N_prueba, v2_prueba, &v3_prueba);
    } else {
        printf("\n  GET ERROR: no existe la clave o falló\n");
    }

    /* Probamos modify_value --> cambiamos los valores de key  */
    char *v1_mod = "valor1_modificado";
    float v2_mod[] = {9.9f, 8.8f};
    struct Paquete v3_mod = {1, 2, 3};

    printf("\nProbamos la función modify_value:\n");

    /* Caso de éxito */
    int err_mod = modify_value(key, v1_mod, 2, v2_mod, v3_mod);
    if (err_mod == 0){
        printf("\nÉxito en modify value\n");
    } else {
        printf("\nError en modify value\n");
    }

    /* Volvemos a hacer get para comprobar el cambio */
    err_get = get_value(key, v1_prueba, &N_prueba, v2_prueba, &v3_prueba);
    if (err_get == 0) {
        printf("GET tras modify OK:\n");
        imprimir(key, v1_prueba, N_prueba, v2_prueba, &v3_prueba);
    } else {
        printf("GET tras modify ERROR\n");
    }

    /* Casos de error en modify_value: N_value2 fuera de rango y clave no existe */
    int err_mod2 = modify_value(key, v1_mod, 150, v2_mod, v3_mod);
    if (err_mod2 == 0){
        printf("\nÉxito en modify value\n");
    } else {
        printf("\nError en modify value (N_value2 fuera de rango)\n");
    }

    int err_mod3 = modify_value("clave_inexistente", v1_mod, 2, v2_mod, v3_mod);
    if (err_mod3 == 0){
        printf("\nÉxito en modify value\n");
    } else {
        printf("\nError en modify value (clave inexistente)\n");
    }

    printf("\nA continuación, pruebas de delete_key:\n");

    /* Caso de éxito: probamos delete_key sobre key_2  */
    int err_del = delete_key(key_2);

    if (err_del == 0){
        printf("\ndelete_key(\"%s\") retorna %d (esperado 0 si existe)\n", key_2, err_del);
    } else {
        printf("Error en delete key\n");
    }

    /* Comprobamos exist(key_2), como ha sido borrada ya no existe */
    if (exist(key_2) == 1){
        printf("La clave \"%s\" existe.\n", key_2);
    } else {
        printf("La clave \"%s\" NO existe.\n", key_2);
    }

    /* Caso de error, no existe la clave a borrar */
    int err_del2 = delete_key("clave_inexistente");
    printf("\nCaso de error: borrar clave inexistente:\n");
    if (err_del2 == 0){
        printf("Éxito borrando la clave\n");
    } else {
        printf("    Error en delete key: clave inexistente.\n");
    }

    /* Probamos destroy y comprobamos que ya no existe key */
    printf("\nPor último, pruebas de destroy:\n\n");

    int err_des = destroy();
    printf("destroy() retorna %d (esperado 0)\n", err_des);

    printf("Comprobamos que ya no existe la clave key (key_2 ya había sido borrada con delete_key):\n");

    if (exist(key) == 1){
        printf("La clave \"%s\" existe.\n", key);
    } else {
        printf("La clave \"%s\" NO existe.\n", key);
    }

    return 0;
}