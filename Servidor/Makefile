# Makefile para compilación de tests
CXX := g++
CXXFLAGS := -std=c++17 -g -Wall -Iinclude -Ilib/xmlrpc
LIBS := -lpthread

# Directorios
BIN_DIR := bin
SRC_DIR := src
TESTS_DIR := tests
INCLUDE_DIR := include
LIB_DIR := lib

# Archivos fuente del proyecto
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
# Archivos de test
TEST_SERIAL_FILE := $(TESTS_DIR)/test_serial_com.cpp
TEST_SERVER_FILE := $(TESTS_DIR)/test_servidor.cpp

# Archivos de XmlRpc++ (desde tu directorio lib/xmlrpc)
XMLRPC_SRC_FILES := $(wildcard $(LIB_DIR)/xmlrpc/*.cpp)
XMLRPC_OBJ_FILES := $(patsubst $(LIB_DIR)/xmlrpc/%.cpp,$(BIN_DIR)/xmlrpc_%.o,$(XMLRPC_SRC_FILES))

# Ejecutables finales
TEST_SERIAL_BIN := $(BIN_DIR)/test_serial_com
TEST_SERVER_BIN := $(BIN_DIR)/test_servidor

# Target principal - compila todo
all: $(TEST_SERIAL_BIN) $(TEST_SERVER_BIN)
	@echo "✅ Todos los tests compilados"

# =============================================
# COMPILACIÓN DE XMLRPC++
# =============================================

# Compilar objetos de XmlRpc++
$(BIN_DIR)/xmlrpc_%.o: $(LIB_DIR)/xmlrpc/%.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =============================================
# COMPILACIÓN DE TESTS INDIVIDUALES
# =============================================

# Test de SerialCom con doctest
$(TEST_SERIAL_BIN): $(SRC_FILES) $(TEST_SERIAL_FILE) $(XMLRPC_OBJ_FILES)
	@echo "🧪 Compilando test de SerialCom..."
	$(CXX) $(CXXFLAGS) $(SRC_FILES) $(TEST_SERIAL_FILE) $(XMLRPC_OBJ_FILES) -o $@ $(LIBS)
	@echo "✅ Test SerialCom compilado: $@"

# Test del servidor XML-RPC
$(TEST_SERVER_BIN): $(SRC_FILES) $(TEST_SERVER_FILE) $(XMLRPC_OBJ_FILES)
	@echo "🌐 Compilando test del servidor..."
	$(CXX) $(CXXFLAGS) $(SRC_FILES) $(TEST_SERVER_FILE) $(XMLRPC_OBJ_FILES) -o $@ $(LIBS)
	@echo "✅ Test Servidor compilado: $@"

# =============================================
# REGLAS DE EJECUCIÓN
# =============================================

# Ejecutar test de SerialCom
test-serial: $(TEST_SERIAL_BIN)
	@echo "🚀 Ejecutando test de SerialCom..."
	./$(TEST_SERIAL_BIN)

# Ejecutar test del servidor (necesita puerto)
test-server: $(TEST_SERVER_BIN)
	@echo "🚀 Ejecutando test del servidor en puerto 8080..."
	./$(TEST_SERVER_BIN) 8080

# Ejecutar todos los tests
test: test-serial test-server

# =============================================
# HERRAMIENTAS DE DESARROLLO
# =============================================

# Limpiar binarios
clean:
	rm -rf $(BIN_DIR)/*
	@echo "🧹 Binarios eliminados"

# Mostrar información del proyecto
info:
	@echo "📊 INFORMACIÓN DEL PROYECTO:"
	@echo "   Archivos fuente: $(words $(SRC_FILES))"
	@echo "   - $(SRC_FILES)"
	@echo "   Archivos XmlRpc++: $(words $(XMLRPC_SRC_FILES))"
	@echo "   - $(XMLRPC_SRC_FILES)"

# Ayuda
help:
	@echo "🎯 COMANDOS DISPONIBLES:"
	@echo "   make all        - Compila todos los tests"
	@echo "   make test-serial - Compila y ejecuta test de SerialCom"
	@echo "   make test-server - Compila y ejecuta test del servidor"
	@echo "   make test       - Ejecuta todos los tests"
	@echo "   make clean      - Limpia binarios"
	@echo "   make info       - Muestra información del proyecto"

.PHONY: all test test-serial test-server clean info help