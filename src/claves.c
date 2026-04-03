#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "linked-list.h"
#include "claves.h"

/* Definiremos head, el puntero al primer elemento de la lista de 
manera global*/

static node_t *head = NULL; // inicialmente lista vacía

int destroy(void){
    // Por ahora no hay errores a comprobar, cuando hagamos la versión distribuida probablemente aparecerán

    node_t *current = head;

    // Si head es NULL, directamente se retorna 0

    while (current != NULL){

        current = current->next;
        free(head);
        head = current;
    }

    return 0;
}


int set_value(char * key, char *value1, int N_value2, float *V_value2, struct Paquete value3){
    
    // Comprobar valores antes de pedir memoria
    if (N_value2 > 32 || N_value2 < 1){
        return -1;
    }

    if (strlen(key) > 255){
        return -1;
    }

    if (strlen(value1) > 255){
        return -1;
    }

    // Comprobar que la clave no exista
    node_t *current = head;

    while (current != NULL){
        if (strcmp(current->key, key) == 0){
            return -1;
        }

        current = current->next;
    }
    

    // ---- Insertar el nuevo nodo ----

    // Crear el nuevo nodo en el heap
    node_t *new_node = (node_t *) malloc(sizeof(node_t));

    if (new_node == NULL){
        printf("Error asignando memoria\n");
        return -1;
    }

    // Rellenar los atributos

    strcpy(new_node->key, key);

    strcpy(new_node->value1, value1);

    new_node->N_value2 = N_value2;

    for (int i = 0; i < N_value2; i++){
        new_node->V_value2[i] = V_value2[i];
    }
    
    new_node->value3 = value3;


    // Insertaremos siempre al principio para O(1)

    new_node->next = head;
    head = new_node;

    return 0;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {

    
    //Empezamos desde el primer nodo
    node_t *current = head;

    while (current != NULL) {

        if (strcmp(current->key, key) == 0){

            //Encontramos la clave que buscamos y se procede a copiar los datos
            strcpy(value1, current->value1);


            *N_value2 = current->N_value2;

            for (int i = 0; i < *N_value2; i++){
                V_value2[i] = current->V_value2[i];
            }

            *value3 = current->value3;

            //Se termina con éxito
            return 0;
        }

        current = current->next;

    }

    return -1;

}


int exist(char *key){
    /* NO ESPECIFICADO POR EL ENUNCIADO:

    if (strlen(key) > 255){
        return -1;
    }
        
    */
    node_t *current = head;

    while (current != NULL){
        if (strcmp(current->key, key) == 0){
            // La clave existe
            return 1;
        }
        current = current->next;
    }
    // La clave no existe
    return 0;
}


int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3){
    
    // Comprobar valores antes de pedir memoria
    if (N_value2 > 32 || N_value2 < 1){
        return -1;
    }

    if (strlen(key) > 255){
        return -1;
    }

    if (strlen(value1) > 255){
        return -1;
    }

    node_t *current = head;

    while (current != NULL) {

        if (strcmp(current->key, key) == 0){

            //Encontramos la clave que buscamos y se procede a copiar los datos
            strcpy(current->value1, value1);

            current->N_value2 = N_value2;


            for (int i = 0; i < N_value2; i++){
                current->V_value2[i] = V_value2[i];
    
            }

            
            current->value3 = value3;

            //Se termina con éxito
            return 0;
        }

        current = current->next;

    }

    return -1;
}

int delete_key(char *key){
    /*Este servicio permite borrar el elemento cuya clave es key. La
función devuelve 0 en caso de éxito y -1 en caso de error. En caso de que la clave no exista
también se devuelve -1.*/

    if (key == NULL) return -1;
    if (head == NULL) return -1;


    node_t *anterior = head;
    node_t *current = anterior->next;
    
    if (strcmp(anterior->key, key)==0){

        head = current;
        free(anterior);

        return 0;
        

        }

    while (current != NULL){

        if (strcmp(current->key, key)==0){
            //Encontramos el elemento qu ehay que borrar
            anterior->next = current->next;
            free(current);

            return 0;
        }

        anterior = current;
        current = current->next;

    }

    return -1;

}

