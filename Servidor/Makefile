CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wno-address -Iinclude -Ilib/xmlrpc
LIBS := -lpthread -lssl -lcrypto -lsqlite3

# Directorios
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
SRC_DIR := src
TESTS_DIR := tests
INCLUDE_DIR := include
LIB_DIR := lib

# Crear directorios necesarios
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR) $(OBJ_DIR)/auth $(OBJ_DIR)/session $(OBJ_DIR)/user $(OBJ_DIR)/db $(OBJ_DIR)/storage $(OBJ_DIR)/services $(OBJ_DIR)/ServiciosRobot)

# ==========================================
# FUENTES DEL PROYECTO
# ==========================================

# Librería XML-RPC
XMLRPC_SRCS := $(wildcard $(LIB_DIR)/xmlrpc/*.cpp)
XMLRPC_OBJS := $(patsubst $(LIB_DIR)/xmlrpc/%.cpp,$(OBJ_DIR)/xmlrpc_%.o,$(XMLRPC_SRCS))

# Core del servidor (sin main) - con rutas corregidas
CORE_SRCS := \
  $(SRC_DIR)/PALogger.cpp \
  $(SRC_DIR)/ServiciosBasicos.cpp \
  $(SRC_DIR)/session/SessionManager.cpp \
  $(SRC_DIR)/session/CurrentUser.cpp \
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
  $(SRC_DIR)/ArduinoService.cpp \
  $(SRC_DIR)/File.cpp \
  $(SRC_DIR)/RobotService.cpp \
  $(SRC_DIR)/TrajectoryManager.cpp \
  $(SRC_DIR)/Servidor.cpp \
  #$(SRC_DIR)/auth/AuthWiring.cpp

# Archivos adicionales necesarios
EXTRA_SRCS := $(wildcard $(SRC_DIR)/ServiciosRobot/*.cpp) \
              $(wildcard $(SRC_DIR)/services/*.cpp) \
              $(wildcard $(SRC_DIR)/storage/*.cpp) \
              $(wildcard $(SRC_DIR)/db/*.cpp)

# Combinar todas las fuentes
ALL_CORE_SRCS := $(CORE_SRCS) $(EXTRA_SRCS)

# Generar objetos manteniendo estructura de directorios
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CORE_SRCS))
EXTRA_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(EXTRA_SRCS))
ALL_CORE_OBJS := $(CORE_OBJS) $(EXTRA_OBJS)

# Main del Servidor
MAIN_SRC := main.cpp
MAIN_OBJ := $(OBJ_DIR)/main.o

# ==========================================
# EJECUTABLES
# ==========================================

# Servidor principal
SERVER_BIN := $(BIN_DIR)/servidor_robot

# Tests individuales
TEST_SERIAL_BIN := $(BIN_DIR)/test_serial_com
TEST_ARDUINO_BIN := $(BIN_DIR)/test_arduino_service
TEST_FILE_BIN := $(BIN_DIR)/test_file
TEST_ROBOT_BIN := $(BIN_DIR)/test_robot_service
TEST_TRAJECTORY_BIN := $(BIN_DIR)/test_trajectory_manager

# ==========================================
# REGLAS DE COMPILACIÓN
# ==========================================

.PHONY: all tests test-serial test-arduino test-servidor clean help run-tests test-pruebita

# Target principal
all: servidor tests
	@echo "✅ Todos los objetivos compilados"

# Servidor principal
servidor: $(SERVER_BIN)
	@echo "✅ Servidor principal compilado"

# Compilar servidor principal
$(SERVER_BIN): $(ALL_CORE_OBJS) $(XMLRPC_OBJS) $(MAIN_OBJ)
	@echo "🚀 Enlazando servidor principal..."
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "✅ Servidor listo: $@"

# Compilar Tests
tests: $(TEST_SERIAL_BIN) $(TEST_ARDUINO_BIN) $(TEST_FILE_BIN) $(TEST_ROBOT_BIN) $(TEST_PRUEBITA_BIN) $(TEST_TRAJECTORY_BIN)
	@echo "✅ Todos los tests compilados"

# =============================================
# COMPILACIÓN DE TESTS INDIVIDUALES
# =============================================

$(TEST_SERIAL_BIN): $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_serial_com.o
	@echo "🧪 Enlazando test de SerialCom..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST_ARDUINO_BIN): $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_arduino_service.o
	@echo "🤖 Enlazando test de ArduinoService..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST_FILE_BIN): $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_file.o
	@echo "📁 Enlazando test de Archivos..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST_ROBOT_BIN): $(CORE_OBJS) $(XMLRPC_OBJS) $(OBJ_DIR)/test_robot_service.o
	@echo "🤖 Enlazando test de RobotService..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST_TRAJECTORY_BIN): $(OBJ_DIR)/TrajectoryManager.o $(OBJ_DIR)/File.o $(OBJ_DIR)/session/CurrentUser.o $(OBJ_DIR)/test_trajectory_manager.o
	@echo "📝 Enlazando test de TrajectoryManager..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# ==========================================
# REGLAS DE PATRÓN PARA COMPILACIÓN
# ==========================================

# Objeto principal (main.cpp en raíz)
$(MAIN_OBJ): $(MAIN_SRC)
	@mkdir -p $(dir $@)
	@echo "🎯 Compilando punto de entrada principal..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Objetos del core (src/) - manteniendo estructura de directorios
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

run: $(SERVER_BIN)
	@echo "🚀 Ejecutando servidor principal..."
	@./$(SERVER_BIN)

run-port: $(SERVER_BIN)
	@echo "🚀 Ejecutando servidor en puerto específico..."
	@./$(SERVER_BIN) 8080

test-serial: $(TEST_SERIAL_BIN)
	@echo "🚀 Ejecutando test de SerialCom..."
	@./$(TEST_SERIAL_BIN)

test-arduino: $(TEST_ARDUINO_BIN)
	@echo "🚀 Ejecutando test de ArduinoService..."
	@./$(TEST_ARDUINO_BIN)

test-file: $(TEST_FILE_BIN)
	@echo "🚀 Ejecutando test de File..."
	@./$(TEST_FILE_BIN)

test-robot: $(TEST_ROBOT_BIN)
	@echo "🚀 Ejecutando test de RobotService..."
	@./$(TEST_ROBOT_BIN)

test-trajectory: $(TEST_TRAJECTORY_BIN)
	@echo "🚀 Ejecutando test de TrajectoryManager..."
	@./$(TEST_TRAJECTORY_BIN)

# Ejecutar todos los tests (sin el servidor)
run-tests: test-serial test-arduino test-file test-robot test-trajectory
	@echo "✅ Todos los tests ejecutados"

# Verificar que los archivos existen antes de compilar
check-files:
	@echo "🔍 Verificando archivos fuente..."
	@for file in $(ALL_CORE_SRCS) $(MAIN_SRC); do \
		if [ ! -f "$$file" ]; then \
			echo "❌ Archivo no encontrado: $$file"; \
			exit 1; \
		fi; \
	done
	@echo "✅ Todos los archivos fuente encontrados"

# ==========================================
# LIMPIEZA Y AYUDA
# ==========================================

clean:
	rm -rf $(BIN_DIR)
	@echo "🧹 Directorio bin eliminado completamente"

# Limpieza parcial (solo objetos)
clean-obj:
	rm -rf $(OBJ_DIR)
	@echo "🧹 Objetos eliminados"

# Recompilar todo desde cero
rebuild: clean all
	@echo "🔄 Recompilación completa terminada"

help:
	@echo "🎯 COMANDOS DISPONIBLES:"
	@echo "   make all               - Compila servidor y todos los tests"
	@echo "   make servidor          - Compila solo el servidor principal"
	@echo "   make tests             - Compila solo los tests"
	@echo "   make run               - Compila y ejecuta el servidor principal"
	@echo "   make run-port          - Compila y ejecuta el servidor en puerto 8080"
	@echo "   make test-serial       - Compila y ejecuta test de SerialCom"
	@echo "   make test-arduino      - Compila y ejecuta test de ArduinoService"
	@echo "   make test-file         - Compila y ejecuta test de Archivos"
	@echo "   make test-robot        - Compila y ejecuta test de RobotService"
	@echo "   make test-pruebita     - Compila y ejecuta pruebita_server"
	@echo "   make run-tests         - Ejecuta todos los tests (sin servidor)"
	@echo "   make check-files       - Verifica que existen los archivos fuente"
	@echo "   make clean             - Limpia completamente el directorio bin"
	@echo "   make clean-obj         - Limpia solo los objetos compilados"
	@echo "   make rebuild           - Limpia y recompila todo"
	@echo "   make help              - Muestra esta ayuda"

# ==========================================
# DEPENDENCIAS AUTOMÁTICAS
# ==========================================

# Incluir dependencias automáticas si existen
-include $(ALL_CORE_OBJS:.o=.d)
-include $(XMLRPC_OBJS:.o=.d)
-include $(MAIN_OBJ:.o=.d)

