#!/bin/bash

# Configurar variables de entorno para que el proxy sepa a dónde ir
export IP_TUPLAS=127.0.0.1
export PORT_TUPLAS=4500

echo "Lanzando 20 clientes simultáneos..."

for i in {1..20}
do
   # El símbolo '&' al final lanza el proceso en segundo plano
   ./cliente > /dev/null 2>&1 & 
   echo "Cliente $i lanzado."
done

# Esperar a que todos los procesos terminen
wait
echo "Todos los clientes han terminado."