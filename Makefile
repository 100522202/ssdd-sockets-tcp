# ============================================================
# Makefile - Ejercicio Evaluable 2 (Sockets TCP)
# ------------------------------------------------------------
# Genera:
#   - libclaves.so        (lógica de almacenamiento key-value)
#   - libproxyclaves.so   (proxy cliente -> servidor por TCP)
#   - servidor            (servicio concurrente)
#   - cliente             (desde src/app-cliente.c)
#   - cliente1            (desde src/app-cliente1.c)
# ============================================================

# Compilador y flags comunes
CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=gnu11 -Iinclude -fPIC

# Flag para que los ejecutables busquen .so en el directorio actual
RPATH_ORIGIN := -Wl,-rpath,'$$ORIGIN'

# Nombres de salida
LIBCLAVES := libclaves.so
LIBPROXY := libproxyclaves.so
SERVER := servidor
CLIENT := cliente
CLIENT1 := cliente1

# Directorios
SRC_DIR := src
BUILD_DIR := build

# Variables de ejecución (puedes sobreescribirlas en línea de comandos)
PORT ?= 4500
IP ?= 127.0.0.1

# Objetos
OBJ_CLAVES := $(BUILD_DIR)/claves.o
OBJ_MENSAJES := $(BUILD_DIR)/mensajes.o
OBJ_PROXY := $(BUILD_DIR)/proxy-sock.o
OBJ_SERVER_MAIN := $(BUILD_DIR)/servidor-sock.o
OBJ_PROCESAR := $(BUILD_DIR)/procesar_funcion.o
OBJ_CLIENT_MAIN := $(BUILD_DIR)/app-cliente.o
OBJ_CLIENT1_MAIN := $(BUILD_DIR)/app-cliente1.o

# Regla por defecto: construir todo
.PHONY: all
all: $(LIBCLAVES) $(LIBPROXY) $(SERVER) $(CLIENT) $(CLIENT1)

# Crear carpeta de objetos
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ----------------------------
# Compilación de objetos (.o)
# ----------------------------
$(OBJ_CLAVES): $(SRC_DIR)/claves.c include/claves.h include/linked-list.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_MENSAJES): $(SRC_DIR)/mensajes.c include/mensajes.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PROXY): $(SRC_DIR)/proxy-sock.c include/claves.h include/mensajes.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_SERVER_MAIN): $(SRC_DIR)/servidor-sock.c include/claves.h include/mensajes.h include/procesar_funcion.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PROCESAR): $(SRC_DIR)/procesar_funcion.c include/procesar_funcion.h include/mensajes.h include/claves.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_CLIENT_MAIN): $(SRC_DIR)/app-cliente.c include/claves.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_CLIENT1_MAIN): $(SRC_DIR)/app-cliente1.c include/claves.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------
# Bibliotecas compartidas (.so)
# ----------------------------
$(LIBCLAVES): $(OBJ_CLAVES)
	$(CC) -shared $^ -o $@

$(LIBPROXY): $(OBJ_PROXY) $(OBJ_MENSAJES)
	$(CC) -shared $^ -o $@

# ----------------------------
# Ejecutables
# ----------------------------
# servidor usa libclaves.so para la lógica de datos
$(SERVER): $(OBJ_SERVER_MAIN) $(OBJ_PROCESAR) $(OBJ_MENSAJES) $(LIBCLAVES)
	$(CC) $(OBJ_SERVER_MAIN) $(OBJ_PROCESAR) $(OBJ_MENSAJES) -L. -lclaves -pthread $(RPATH_ORIGIN) -o $@

# cliente y cliente1 usan libproxyclaves.so (la API remota)
$(CLIENT): $(OBJ_CLIENT_MAIN) $(LIBPROXY)
	$(CC) $(OBJ_CLIENT_MAIN) -L. -lproxyclaves $(RPATH_ORIGIN) -o $@

$(CLIENT1): $(OBJ_CLIENT1_MAIN) $(LIBPROXY)
	$(CC) $(OBJ_CLIENT1_MAIN) -L. -lproxyclaves $(RPATH_ORIGIN) -o $@

# ----------------------------
# Limpieza
# ----------------------------
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SERVER) $(CLIENT) $(CLIENT1)

.PHONY: distclean
distclean: clean
	rm -f $(LIBCLAVES) $(LIBPROXY)

# ----------------------------
# Ayuda rápida
# ----------------------------
.PHONY: help
help:
	@echo "Objetivos disponibles:"
	@echo "  make            -> compila bibliotecas + servidor + clientes"
	@echo "  make run-server [PORT=4500]                       -> ejecuta servidor desde la raiz"
	@echo "  make run-client [IP=127.0.0.1] [PORT=4500]       -> ejecuta cliente desde la raiz"
	@echo "  make run-client1 [IP=127.0.0.1] [PORT=4500]      -> ejecuta cliente1 desde la raiz"
	@echo "  make clean      -> elimina objetos y ejecutables"
	@echo "  make distclean  -> clean + elimina bibliotecas .so"
	@echo "  make help       -> muestra esta ayuda"

.PHONY: run-server run-client run-client1
run-server: $(SERVER)
	./$(SERVER) $(PORT)

run-client: $(CLIENT)
	env IP_TUPLAS=$(IP) PORT_TUPLAS=$(PORT) ./$(CLIENT)

run-client1: $(CLIENT1)
	env IP_TUPLAS=$(IP) PORT_TUPLAS=$(PORT) ./$(CLIENT1)
