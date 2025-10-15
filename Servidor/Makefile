# Makefile que compila TODO automáticamente
CXX := g++
CXXFLAGS := -std=c++17 -g -Wall -Iinclude -Ilib/xmlrpc
LIBS := -lpthread

# Encuentra TODOS los archivos .cpp automáticamente
CPP_FILES := $(shell find . -name "*.cpp")

servidor: $(CPP_FILES)
	@echo "Compilando $(words $(CPP_FILES)) archivos .cpp encontrados..."
	$(CXX) $(CXXFLAGS) $(CPP_FILES) -o servidor $(LIBS)
	@echo "✅ Servidor compilado!"
	@echo "Ejecuta: ./servidor 8080"

clean:
	rm -f servidor servidor.log
	@echo "✅ Limpieza completada"

run: servidor
	./servidor 8080

# Esta regla muestra qué archivos se encontraron (para debugging)
debug:
	@echo "Archivos .cpp encontrados:"
	@find . -name "*.cpp" | sort
	@echo "Total: $(words $(CPP_FILES)) archivos"

.PHONY: clean run debug