# ==========================================
#  Proyecto: Integrador POO (Servidor)
#  Ruta: Servidor/
# ==========================================

# --- toolchain ---
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall
INCLUDES := -Iinclude -Ilib/xmlrpc

# ==========================================
#  Fuentes comunes del servidor
# ==========================================
# Librería XML-RPC (todos los .cpp dentro de lib/xmlrpc)
XMLRPC_SRCS := $(wildcard lib/xmlrpc/*.cpp)

# Núcleo del servidor y módulos auth/user/session
CORE_SRCS := \
  src/PALogger.cpp \
  src/ServiciosBasicos.cpp \
  src/session/SessionManager.cpp \
  src/user/UserEntity.cpp \
  src/user/UsersRepoCsv.cpp \
  src/auth/AuthLogin.cpp \
  src/auth/AuthMe.cpp \
  src/auth/AuthLogout.cpp \
  src/user/UserRegister.cpp \
  src/user/UserUpdate.cpp \
  src/user/UserChangePassword.cpp \
  src/user/UserList.cpp

# ==========================================
#  Ejecutable del servidor
#  (usa SOLO un main: tests/test_servidor.cpp)
# ==========================================
SERVER_MAIN := tests/test_servidor.cpp
SERVER_SRCS := $(CORE_SRCS) $(XMLRPC_SRCS) $(SERVER_MAIN)
SERVER_OBJS := $(SERVER_SRCS:.cpp=.o)
TARGET      := servidor_min

# ==========================================
#  Ejecutables de tests (doctest)
#  Cada test lleva su propio main (DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
#  Por eso: un binario por archivo de test.
# ==========================================
# Módulos que necesitan los tests
SERIAL_SRCS  := src/SerialCom.cpp
ARDUINO_SRCS := src/ArduinoService.cpp

TESTS := test_serial_com test_arduino_service

# test_serial_com
TEST_serial_com_SRCS := $(CORE_SRCS) $(XMLRPC_SRCS) $(SERIAL_SRCS) tests/test_serial_com.cpp
TEST_serial_com_OBJS := $(TEST_serial_com_SRCS:.cpp=.o)

# test_arduino_service
TEST_arduino_service_SRCS := $(CORE_SRCS) $(XMLRPC_SRCS) $(SERIAL_SRCS) $(ARDUINO_SRCS) tests/test_arduino_service.cpp
TEST_arduino_service_OBJS := $(TEST_arduino_service_SRCS:.cpp=.o)

# ==========================================
#  Targets por defecto
# ==========================================
.PHONY: all tests clean mrproper info help

all: $(TARGET)

tests: $(TESTS)

# Servidor
$(TARGET): $(SERVER_OBJS)
	@echo "🔧 Enlazando $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJS)

# Tests
test_serial_com: $(TEST_serial_com_OBJS)
	@echo "🔧 Enlazando $@..."
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_serial_com_OBJS)

test_arduino_service: $(TEST_arduino_service_OBJS)
	@echo "🔧 Enlazando $@..."
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_arduino_service_OBJS)

# Compilación genérica
%.o: %.cpp
	@echo "🧩 Compilando $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Silenciar warnings de -Waddress SOLO en archivos de tests (no en el resto)
tests/%.o: CXXFLAGS += -Wno-address

# Limpieza
clean:
	@echo "🧹 Limpiando objetos..."
	rm -f $(SERVER_OBJS) $(TEST_serial_com_OBJS) $(TEST_arduino_service_OBJS)

mrproper: clean
	@echo "🗑️  Borrando ejecutables..."
	rm -f $(TARGET) $(TESTS)

# Info / ayuda
info:
	@echo "Servidor fuentes: $(words $(SERVER_SRCS)) archivos"
	@echo "Tests: $(TESTS)"

help:
	@echo "make            -> compila servidor_min"
	@echo "make tests      -> compila todos los tests (binarios separados)"
	@echo "make clean      -> borra .o"
	@echo "make mrproper   -> borra .o y ejecutables"

