// --- Servidor/tests/test_trajectory_manager.cpp ---

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/doctest.h"
#include "../include/TrajectoryManager.h"
#include "../include/File.h"  // Necesario para leer/verificar
#include <filesystem>
#include <vector>
#include <string>

// --- Configuración de la Prueba ---
const std::string DIRECTORIO_PRUEBAS = "data/trayectorias_test/";
const std::string NOMBRE_PRUEBA_LOGICO = "mi_prueba_basica";

// --- Helper para limpiar (opcional pero recomendado) ---
// Nos aseguramos de empezar con un directorio limpio
void limpiarDirectorioTest() {
    std::error_code ec;
    std::filesystem::remove_all(DIRECTORIO_PRUEBAS, ec);
    if (ec) {
        std::cerr << "Error limpiando directorio: " << ec.message() << std::endl;
    }
    std::filesystem::create_directories(DIRECTORIO_PRUEBAS, ec);
    if (ec) {
        std::cerr << "Error creando directorio: " << ec.message() << std::endl;
    }
}

// ===========================================
// --- TEST CASE PRINCIPAL ---
// ===========================================
TEST_CASE("TrajectoryManager: Flujo completo de grabación") {
    
    limpiarDirectorioTest();
    
    // 1. CREACIÓN
    // Usamos el constructor que recibe el directorio base
    TrajectoryManager manager(DIRECTORIO_PRUEBAS);
    CHECK(manager.getDirectorioBase() == DIRECTORIO_PRUEBAS);
    
    // Estado inicial: no debería estar grabando
    CHECK_FALSE(manager.estaGrabando());

    // 2. INICIAR GRABACIÓN
    // Le pasamos un nombre "lógico"
    bool inicioOk = manager.iniciarGrabacion(NOMBRE_PRUEBA_LOGICO);
    
    // Verificaciones de estado
    CHECK(inicioOk);
    CHECK(manager.estaGrabando());
    
    // Guardamos el nombre real del archivo (que incluye ID y timestamp)
    std::string nombreArchivoReal = manager.getTrayectoriaActual();
    CHECK_FALSE(nombreArchivoReal.empty());
    CHECK(nombreArchivoReal.rfind(".gcode") != std::string::npos); // Debería terminar en .gcode

    // 3. GUARDAR COMANDOS
    CHECK(manager.guardarComando("G1 X100 Y50 Z10"));
    CHECK(manager.guardarComando("M3")); // Activar efector
    CHECK(manager.guardarComando("G1 X100 Y50 Z-20"));
    CHECK(manager.guardarComando("M5")); // Desactivar efector

    // 4. FINALIZAR GRABACIÓN
    CHECK(manager.finalizarGrabacion());
    CHECK_FALSE(manager.estaGrabando());
    CHECK(manager.getTrayectoriaActual().empty()); // Debería limpiarse el nombre

    // 5. VERIFICACIÓN (El paso más importante)
    // Leemos el archivo que acabamos de crear usando el manager
    std::vector<std::string> lineas = manager.cargarTrayectoria(nombreArchivoReal);
    
    // Verificamos que el contenido sea el esperado
    REQUIRE(lineas.size() == 4); // Requerimos 4 líneas
    CHECK(lineas[0] == "G1 X100 Y50 Z10");
    CHECK(lineas[1] == "M3");
    CHECK(lineas[2] == "G1 X100 Y50 Z-20");
    CHECK(lineas[3] == "M5");

    // 6. LIMPIEZA (opcional si 'limpiarDirectorioTest' se llama al inicio)
    CHECK(manager.eliminarTrayectoria(nombreArchivoReal));
    CHECK_FALSE(manager.existeTrayectoria(nombreArchivoReal));
}

TEST_CASE("TrajectoryManager: Casos Borde") {
    
    limpiarDirectorioTest();
    TrajectoryManager manager(DIRECTORIO_PRUEBAS);

    // 1. Guardar sin iniciar
    CHECK_FALSE(manager.guardarComando("G1 X0 Y0")); // No debería guardar nada

    // 2. Iniciar dos veces
    CHECK(manager.iniciarGrabacion("prueba_doble"));
    CHECK(manager.estaGrabando());
    
    // El segundo intento debe fallar
    CHECK_FALSE(manager.iniciarGrabacion("otra_prueba")); 
    
    // Verificamos que sigue grabando la *primera*
    CHECK(manager.estaGrabando());
    CHECK(manager.getTrayectoriaActual().rfind("prueba_doble", 0) != std::string::npos);

    manager.finalizarGrabacion();
}