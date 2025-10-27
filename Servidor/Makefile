CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Iinclude -Ilib/xmlrpc
LIBS := -lpthread

# Directorios
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
SRC_DIR := src
TESTS_DIR := tests
INCLUDE_DIR := include
LIB_DIR := lib

# Crear directorios necesarios
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR))

# ==========================================
# FUENTES DEL PROYECTO
# ==========================================

# Librería XML-RPC
XMLRPC_SRCS := $(wildcard $(LIB_DIR)/xmlrpc/*.cpp)
XMLRPC_OBJS := $(patsubst $(LIB_DIR)/xmlrpc/%.cpp,$(OBJ_DIR)/xmlrpc_%.o,$(XMLRPC_SRCS))

# Core del servidor (sin main)
CORE_SRCS := \
  $(SRC_DIR)/PALogger.cpp \
  $(SRC_DIR)/ServiciosBasicos.cpp \
  $(SRC_DIR)/session/SessionManager.cpp \
  $(SRC_DIR)/user/UserEntity.cpp \
  $(SRC_DIR)/user/UsersRepoCsv.cpp \
  $(SRC_DIR)/auth/AuthLogin.cpp \
  $(SRC_DIR)/auth/AuthMe.cpp \
  $(SRC_DIR)/auth/AuthLogout.cpp \
  $(SRC_DIR)/user/UserRegister.cpp \
  $(SRC_DIR)/user/UserUpdate.cpp \
  $(SRC_DIR)/user/UserChangePassword.cpp \
  $(SRC_DIR)/user/UserList.cpp \
  $(SRC_DIR)/SerialCom.cpp \
  $(SRC_DIR)/ArduinoService.cpp

CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CORE_SRCS))

# ==========================================
# EJECUTABLES
# ==========================================

# Servidor principal
SERVER_MAIN := $(TESTS_DIR)/test_servidor.cpp
SERVER_OBJS := $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_servidor.o
SERVER_BIN := $(BIN_DIR)/servidor_min

# Tests individuales
TEST_SERIAL_BIN := $(BIN_DIR)/test_serial_com
TEST_ARDUINO_BIN := $(BIN_DIR)/test_arduino_service

# ==========================================
# REGLAS DE COMPILACIÓN
# ==========================================

.PHONY: all tests test-serial test-arduino test-servidor clean help

# Target principal
all: $(SERVER_BIN) tests
	@echo "✅ Todos los objetivos compilados"

# Compilar servidor principal
$(SERVER_BIN): $(SERVER_OBJS)
	@echo "🔧 Enlazando servidor..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "✅ Servidor compilado: $@"

# Tests
tests: $(TEST_SERIAL_BIN) $(TEST_ARDUINO_BIN)
	@echo "✅ Tests compilados"

$(TEST_SERIAL_BIN): $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_serial_com.o
	@echo "🧪 Enlazando test de SerialCom..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST_ARDUINO_BIN): $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_arduino_service.o
	@echo "🤖 Enlazando test de ArduinoService..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# ==========================================
# REGLAS DE PATRÓN PARA COMPILACIÓN
# ==========================================

# Objetos del core (src/)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "🧩 Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Objetos de XML-RPC
$(OBJ_DIR)/xmlrpc_%.o: $(LIB_DIR)/xmlrpc/%.cpp
	@mkdir -p $(dir $@)
	@echo "🧩 Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Objetos de tests
$(OBJ_DIR)/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "🧩 Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# REGLAS DE EJECUCIÓN
# ==========================================

test-serial: $(TEST_SERIAL_BIN)
	@echo "🚀 Ejecutando test de SerialCom..."
	./$(TEST_SERIAL_BIN)

test-arduino: $(TEST_ARDUINO_BIN)
	@echo "🚀 Ejecutando test de ArduinoService..."
	./$(TEST_ARDUINO_BIN)

test-servidor: $(SERVER_BIN)
	@echo "🚀 Ejecutando servidor en puerto 8080..."
	./$(SERVER_BIN) 8080

test: test-serial test-arduino
	@echo "⏳ Esperando para test del servidor..."
	@sleep 2
	@echo "🚀 Ejecutando test del servidor en puerto 8080..."
	@./$(SERVER_BIN) 8080

# ==========================================
# LIMPIEZA Y AYUDA
# ==========================================

clean:
	rm -rf $(BIN_DIR)
	@echo "🧹 Directorio bin eliminado completamente"

help:
	@echo "🎯 COMANDOS DISPONIBLES:"
	@echo "   make all         - Compila servidor y todos los tests"
	@echo "   make tests       - Compila solo los tests"
	@echo "   make test-serial - Compila y ejecuta test de SerialCom"
	@echo "   make test-arduino - Compila y ejecuta test de ArduinoService"
	@echo "   make test-servidor - Compila y ejecuta el servidor"
	@echo "   make test        - Ejecuta todos los tests"
	@echo "   make clean       - Limpia completamente el directorio bin"
	@echo "   make help        - Muestra esta ayuda"

