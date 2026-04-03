#ifndef LINKED_LIST
#define LINKED_LIST

#include "claves.h"
#include <inttypes.h>

/* Se elegirá una lista enlazada como estructura de datos:
    - Un nodo contiene los datos útiles + puntero al siguiente nodo   
*/

typedef struct node{
    char key[256]; // 255 + '/0'
    char value1[256];
    float V_value2[32];
    int32_t N_value2; // 1 <= N <= 32
    struct Paquete value3;

    struct node * next;
} node_t;

/* Las operaciones asociadas se implementarán en claves.c */

#endif