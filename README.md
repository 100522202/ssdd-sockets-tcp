# Servicio Clave-Valor Distribuido sobre Sockets TCP - Sistemas Distribuidos (SSDD)

Ejercicio Evaluable 2 de la asignatura **Sistemas Distribuidos (UC3M)**.

---

## Descripción

Evolución del servicio clave-valor distribuido para operar sobre redes de comunicación utilizando **Sockets TCP en C**.

### Características:
* **Servidor TCP Multihilo:** Escucha en una IP y puerto configurables mediante variables de entorno (`IP_SERVIDOR`, `PUERTO_SERVIDOR`) y gestiona clientes en hilos separados.
* **Control de Flujo y Fragmentación:** Rutinas auxiliares robustas de lectura (`read_all`) y escritura (`write_all`) para garantizar la recepción íntegra de paquetes sin problemas de fragmentación TCP.
* **Serialización Binaria:** Empaquetado y desempaquetado de cadenas de texto y vectores de números reales de tamaño dinámico.

---

## Compilación y Ejecución

```bash
make

# Terminal 1: Iniciar servidor
export PUERTO_SERVIDOR=8080
./servidor-sock $PUERTO_SERVIDOR

# Terminal 2: Ejecutar cliente
export IP_SERVIDOR=127.0.0.1
export PUERTO_SERVIDOR=8080
./app-cliente
```
