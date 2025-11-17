#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "lib/xmlrpc/doctest.h"
#include "hardware/ArduinoService.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

// Variables globales para la conexión persistente
static std::unique_ptr<ArduinoService> arduinoService = nullptr;
static bool conexionInicializada = false;

TEST_SUITE("ArduinoService Integration Tests") {

    struct TestGlobalSetup {
        TestGlobalSetup() {
            if (!conexionInicializada) {
                std::cout << "🔄 INICIALIZACIÓN GLOBAL - Conectando al Arduino..." << std::endl;
                arduinoService = std::make_unique<ArduinoService>("/dev/ttyUSB0", 115200);
                    
                if (arduinoService->conectar(3)) {
                    std::cout << "✅ CONEXIÓN GLOBAL ESTABLECIDA" << std::endl;
                    
                    // Esperar a que el Arduino se estabilice después del reinicio
                    std::this_thread::sleep_for(3000ms);
                        
                    // Limpiar buffer inicial
                    try {
                        arduinoService->enviarComando("\r\n", 1000ms); // Comando vacío para limpiar
                    } catch (...) {}
                        
                    // Enviar G28 requerido
                    std::cout << "🔧 Enviando homing inicial (G28)..." << std::endl;
                    try {
                        std::string respuesta = arduinoService->enviarComando("G28\r\n", 10000ms);
                        std::cout << "✅ Homing inicial completado:\r\n" << respuesta << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "❌ Error en homing inicial:\r\n" << e.what() << std::endl;
                    }
                        
                    conexionInicializada = true;
                } else {
                    std::cout << "❌ No se pudo establecer conexión global" << std::endl;
                }
            }
        }
    };

    TEST_CASE("Inicializacion") {
        TestGlobalSetup setup;

        if (conexionInicializada && arduinoService->estaConectado()) {
            CHECK(arduinoService->getPuerto() == "/dev/ttyUSB0");
            CHECK(arduinoService->getBaudrate() == 115200);
            CHECK(arduinoService->estaConectado());
        } else {
            WARN("Arduino no conectado - tests omitidos");
        }
    }

    //
    // ======== G-CODE COMMANDS - MOVEMENT ========
    //

    TEST_CASE("G-code Commands - Movement") {
        TestGlobalSetup setup;
        
        if (!conexionInicializada || !arduinoService->estaConectado()) {
            WARN("Arduino no conectado - tests de movimiento omitidos");
            return;
        }

        SUBCASE("Absolute positioning (G90)") {
            std::string respuesta = arduinoService->enviarComando("G90\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta G90:\r\n" << respuesta << std::endl;
        }

        SUBCASE("Linear move (G1)") {
            std::string respuesta = arduinoService->enviarComando("G1 X100 Y50 Z20 F300\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta G1:\r\n" << respuesta << std::endl;
        }

        SUBCASE("Relative move sequence") {
            // Modo relativo
            std::string respuesta = arduinoService->enviarComando("G91\r\n");
            CHECK_FALSE(respuesta.empty());
            
            // Movimiento relativo
            respuesta = arduinoService->enviarComando("G1 X10 Y-5 Z2 F100\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta G1 relativo:\r\n" << respuesta << std::endl;
            
            // Volver a modo absoluto
            arduinoService->enviarComando("G90\r\n");
        }

        SUBCASE("Set position (G92)") {
            std::string respuesta = arduinoService->enviarComando("G92 X0 Y0 Z0\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta G92:\r\n" << respuesta << std::endl;
        }
    }

    //
    // ======== M-CODE COMMANDS - SYSTEM CONTROL ========
    //

    TEST_CASE("M-code Commands - System Control") {
        TestGlobalSetup setup;
        
        if (!conexionInicializada || !arduinoService->estaConectado()) {
            WARN("Arduino no conectado - tests M-code omitidos");
            return;
        }

        SUBCASE("Enable motors (M17)") {
            std::string respuesta = arduinoService->enviarComando("M17\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta M17:\r\n" << respuesta << std::endl;
        }

        SUBCASE("Get current position (M114)") {
            std::string respuesta = arduinoService->enviarComando("M114\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Posición actual (M114):\r\n" << respuesta << std::endl;
        }

        SUBCASE("Get endstop status (M119)") {
            std::string respuesta = arduinoService->enviarComando("M119\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Estado endstops (M119):\r\n" << respuesta << std::endl;
        }

        SUBCASE("Fan control") {
            std::string respuesta = arduinoService->enviarComando("M106\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta M106:\r\n" << respuesta << std::endl;

            respuesta = arduinoService->enviarComando("M107\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta M107:\r\n" << respuesta << std::endl;
        }

        SUBCASE("Disable motors (M18)") {
            std::string respuesta = arduinoService->enviarComando("M18\r\n");
            CHECK_FALSE(respuesta.empty());
            std::cout << "Respuesta M18:\r\n" << respuesta << std::endl;
        }
    }

    //
    // ======== SECUENCIA DE OPERACIONES ========
    //

    TEST_CASE("Sequential Operations") {
        TestGlobalSetup setup;
        
        if (!conexionInicializada || !arduinoService->estaConectado()) {
            WARN("Arduino no conectado - tests de secuencia omitidos");
            return;
        }

        std::cout << "🧪 Ejecutando secuencia de operaciones..." << std::endl;

        // Secuencia completa
        arduinoService->enviarComando("M17\r\n");  // Habilitar motores
        arduinoService->enviarComando("G90\r\n");  // Modo absoluto
        
        // Movimiento a posición inicial
        std::string resp1 = arduinoService->enviarComando("G1 X50 Y50 Z30 F200\r\n");
        CHECK_FALSE(resp1.empty());
        
        // Activar gripper
        std::string resp2 = arduinoService->enviarComando("M3\r\n");
        CHECK_FALSE(resp2.empty());
        
        // Movimiento con carga
        std::string resp3 = arduinoService->enviarComando("G1 X100 Y75 Z40 F150\r\n");
        CHECK_FALSE(resp3.empty());
        
        // Desactivar gripper
        std::string resp4 = arduinoService->enviarComando("M5\r\n");
        CHECK_FALSE(resp4.empty());
        
        std::cout << "✅ Secuencia completada exitosamente" << std::endl;
    }
}

// Función para limpiar la conexión global al final de todos los tests
struct TestGlobalTeardown {
    ~TestGlobalTeardown() {
        if (arduinoService && arduinoService->estaConectado()) {
            std::cout << "🧹 LIMPIANDO CONEXIÓN GLOBAL..." << std::endl;
            // Enviar comando de seguridad antes de desconectar
            try {
                arduinoService->enviarComando("M18\r\n");  // Deshabilitar motores
                arduinoService->enviarComando("G90\r\n");  // Volver a modo absoluto
                arduinoService->enviarComando("G1 X0 Y0 Z50 F300\r\n");  // Posición segura
            } catch (...) {}
            
            arduinoService->desconectar();
            std::cout << "✅ CONEXIÓN GLOBAL CERRADA" << std::endl;
        }
    }
};

// Instancia global que se destruirá al final del programa
static TestGlobalTeardown globalTeardown;
