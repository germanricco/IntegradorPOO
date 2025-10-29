// test_robot_service.cpp - Tests actualizados
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "RobotService.h"
#include "PALogger.h"
#include "ArduinoService.h"
#include <memory>
#include <chrono>
#include <iomanip>

// Configuración global para tests
struct TestConfig {
    static const std::string TEST_PORT;
    static const int TEST_BAUDRATE;
    static const std::string LOG_FILENAME;
};

const std::string TestConfig::TEST_PORT = "/dev/ttyACM0";
const int TestConfig::TEST_BAUDRATE = 115200;
const std::string TestConfig::LOG_FILENAME = "log_test_robot_service.log";

// Fixture global que se inicializa una única vez
struct GlobalTestFixture {
    static std::shared_ptr<PALogger> logger;
    static std::shared_ptr<ArduinoService> arduinoService;
    static std::shared_ptr<RobotService> robotService;
    static bool conexionInicializada;

    GlobalTestFixture() {
        if (!conexionInicializada) {
            std::cout << "🔄 INICIALIZANDO FIXTURE GLOBAL..." << std::endl;
            
            // Inicializar logger (usando tu constructor exacto)
            logger = std::make_shared<PALogger>(
                LogLevel::INFO,             // Nivel de log
                true,                      // logToFile
                TestConfig::LOG_FILENAME   // filename
            );
            
            logger->info("=== INICIO TESTS ROBOTSERVICE ===");
            logger->info("Inicializando servicios...");
            
            // Inicializar servicios
            arduinoService = std::make_shared<ArduinoService>(
                TestConfig::TEST_PORT, 
                TestConfig::TEST_BAUDRATE
            );
            
            // Instancia de RobotService
            robotService = std::make_shared<RobotService>(
                arduinoService, 
                *logger
            );
            
            // Intentar conexión una única vez
            logger->info("Conectando al Arduino...");
            bool conectado = robotService->conectarRobot(3);
            
            if (conectado) {
                logger->info("✅ CONEXIÓN GLOBAL ESTABLECIDA");
                conexionInicializada = true;
                
                // Configuración inicial del robot
                logger->info("Activando Motores...");
                robotService->activarMotores();
                
            } else {
                logger->info("❌ NO SE PUDO ESTABLECER CONEXIÓN GLOBAL");
            }
            
            std::cout << "✅ FIXTURE GLOBAL INICIALIZADO" << std::endl;
        }
    }
    
    ~GlobalTestFixture() {
        if (conexionInicializada && robotService) {
            logger->info("=== FIN TESTS ROBOTSERVICE ===");
            logger->info("Desconectando robot...");
            robotService->desconectarRobot();
        }
    }
};

// Definición de variables estáticas
std::shared_ptr<PALogger> GlobalTestFixture::logger = nullptr;
std::shared_ptr<ArduinoService> GlobalTestFixture::arduinoService = nullptr;
std::shared_ptr<RobotService> GlobalTestFixture::robotService = nullptr;
bool GlobalTestFixture::conexionInicializada = false;

// Instancia global del fixture - se crea antes de cualquier test
static GlobalTestFixture GLOBAL_FIXTURE;

TEST_SUITE("RobotService Unit Tests") {
    
    TEST_CASE("Inicialización de servicios") {
        // Usar los servicios del fixture global
        auto& logger = GlobalTestFixture::logger;
        auto& robotService = GlobalTestFixture::robotService;
        
        REQUIRE(logger != nullptr);
        REQUIRE(robotService != nullptr);
        
        CHECK(robotService->getModoOperacion() == RobotService::ModoOperacion::MANUAL);
        CHECK(robotService->getModoCoordenadas() == RobotService::ModoCoordenadas::ABSOLUTO);
    }
}

TEST_SUITE("RobotService Integration Tests") {
    
    TEST_CASE("Verificación de conexión") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        REQUIRE(robotService != nullptr);
        
        if (GlobalTestFixture::conexionInicializada) {
            CHECK(robotService->estaConectado());
            logger->info("✅ Conexión verificada exitosamente");
        } else {
            WARN("⚠️  Arduino no disponible - Tests de integración serán omitidos");
        }
    }
    
    TEST_CASE("Comando G28 - Homing") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("🧪 TEST: Comando G28 - Homing");
        std::string respuesta = robotService->homing();
        
        CHECK_FALSE(respuesta.empty());
        CHECK(robotService->getModoEjecucion() == RobotService::ModoEjecucion::DETENIDO);

        logger->info("✅ Homing Rta: " + respuesta);
    }
    
    TEST_CASE("Comando G1 - Movimiento") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("🧪 TEST: Comando G1 - Movimiento");
        
        SUBCASE("Movimiento con velocidad") {
            std::string respuesta = robotService->mover(100.0, 50.0, 50.0, 100.0);
            CHECK_FALSE(respuesta.empty());
            bool tieneError = (respuesta.find("ERROR:") == 0);
            CHECK_FALSE(tieneError);
            logger->info("✅ Movimiento con velocidad: " + respuesta);
        }
        
        SUBCASE("Movimiento con velocidad por defecto") {
            std::string respuesta = robotService->mover(105.0, 55.0, 45.0);
            CHECK_FALSE(respuesta.empty());
            bool tieneError = (respuesta.find("ERROR:") == 0);
            CHECK_FALSE(tieneError);
            logger->info("✅ Rta. movimiento sin velocidad: " + respuesta);
        }
        
        SUBCASE("Movimiento inválido") {
            std::string respuesta = robotService->mover(1000.0, 1000.0, 1000.0); // Fuera de límites
            bool tieneError = (respuesta.find("ERROR:") == 0);
            CHECK_FALSE(tieneError);
            logger->info("✅ Movimiento inválido detectado: " + respuesta);
        }
    }
    
    TEST_CASE("Comandos M3/M5 - Efector Final") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("🧪 TEST: Comandos M3/M5 - Efector Final");
        
        SUBCASE("Activar efector") {
            std::string respuesta = robotService->activarEfector();
            CHECK_FALSE(respuesta.empty());
            logger->info("✅ Efector activado: " + respuesta);
        }
        
        SUBCASE("Desactivar efector") {
            std::string respuesta = robotService->desactivarEfector();
            CHECK_FALSE(respuesta.empty());
            logger->info("✅ Efector desactivado: " + respuesta);
        }
    }
    
    TEST_CASE("Comandos M17/M18 - Control de Motores") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("🧪 TEST: Comandos M17/M18 - Control de Motores");
        
        SUBCASE("Desactivar motores") {
            std::string respuesta = robotService->desactivarMotores();
            CHECK_FALSE(respuesta.empty());
            logger->info("✅ Motores desactivados: " + respuesta);
        }
        
        SUBCASE("Activar motores") {
            std::string respuesta = robotService->activarMotores();
            CHECK_FALSE(respuesta.empty());
            logger->info("✅ Motores activados: " + respuesta);
        }
    }
    
    TEST_CASE("Comando M114 - Estado del Robot") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("🧪 TEST: Comando M114 - Estado del Robot");
        std::string respuesta = robotService->obtenerEstado();
        
        CHECK_FALSE(respuesta.empty());
        logger->info("✅ Estado obtenido: " + respuesta);
    }
    
    TEST_CASE("Comandos G90/G91 - Modos Coordenadas") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("🧪 TEST: Comandos G90/G91 - Modos Coordenadas");
        
        SUBCASE("Modo relativo G91") {
            bool exito = robotService->setModoCoordenadas(RobotService::ModoCoordenadas::RELATIVO);
            CHECK(exito);
            CHECK(robotService->getModoCoordenadas() == RobotService::ModoCoordenadas::RELATIVO);
            logger->info("✅ Modo relativo configurado");
        }
        
        SUBCASE("Modo absoluto G90") {
            bool exito = robotService->setModoCoordenadas(RobotService::ModoCoordenadas::ABSOLUTO);
            CHECK(exito);
            CHECK(robotService->getModoCoordenadas() == RobotService::ModoCoordenadas::ABSOLUTO);
            logger->info("✅ Modo absoluto configurado");
        }
    }
    
    TEST_CASE("Secuencia Completa de Operación") {
        auto& robotService = GlobalTestFixture::robotService;
        auto& logger = GlobalTestFixture::logger;
        
        if (!GlobalTestFixture::conexionInicializada) {
            WARN("Arduino no conectado - Test omitido");
            return;
        }
        
        logger->info("\n🧪 TEST: Secuencia Completa de Operación");
        logger->info("🔧 INICIANDO SECUENCIA COMPLETA\n");
        
        // 1. Homing
        std::string respuesta = robotService->homing();
        logger->info("✅ Paso 1/7 - Homing completado");

        // 2. Activar motores (por si se desactivaron)
        respuesta = robotService->activarMotores();
        CHECK(respuesta.find("ERROR") == std::string::npos);
        logger->info("✅ Paso 2/7 - Motores activados");
        
        // 3. Movimiento a posición inicial
        respuesta = robotService->mover(100, 100, 100, 200);
        CHECK(respuesta.find("ERROR") == std::string::npos);
        logger->info("✅ Paso 3/7 - Movimiento a posición inicial");
        
        // 4. Activar efector
        respuesta = robotService->activarEfector();
        CHECK(respuesta.find("ERROR") == std::string::npos);
        logger->info("✅ Paso 4/7 - Efector activado");
        
        // 5. Movimiento con efector activado
        respuesta = robotService->mover(100, 100, 30, 100);
        CHECK(respuesta.find("ERROR") == std::string::npos);
        logger->info("✅ Paso 5/7 - Movimiento con efector activado");
        
        // 6. Desactivar efector
        respuesta = robotService->desactivarEfector();
        CHECK(respuesta.find("ERROR") == std::string::npos);
        logger->info("✅ Paso 6/7 - Efector desactivado");
        
        // 7. Obtener estado final
        respuesta = robotService->obtenerEstado();
        CHECK(respuesta.find("ERROR") == std::string::npos);
        logger->info("✅ Paso 7/7 - Estado final obtenido");
        
        logger->info("🎉 SECUENCIA COMPLETA FINALIZADA EXITOSAMENTE");
    }
}