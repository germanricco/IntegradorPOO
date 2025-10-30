#include <iostream>
#include <csignal>
#include <atomic>
#include <cstdlib>
#include <memory> // Para std::shared_ptr y std::make_shared
#include "XmlRpc.h"

// === HERRAMIENTAS GLOBALES ===
#include "../include/PALogger.h"
#include "../include/ArduinoService.h"
#include "../include/TrajectoryManager.h"
#include "../include/RobotService.h" // La fachada clave

// sesiones / usuarios
#include "../include/session/SessionManager.h"
#include "../include/user/UsersRepoCsv.h"

// auth.*
#include "../include/auth/AuthLogin.h"
#include "../include/auth/AuthMe.h"
#include "../include/auth/AuthLogout.h"

// user.*
#include "../include/user/UserRegister.h"
#include "../include/user/UserUpdate.h"
#include "../include/user/UserChangePassword.h"
#include "../include/user/UserList.h"

// Servicios Robot
#include "../include/ServiciosRobot/RobotHomingMethod.h" // <-- NUESTRO PRIMER SERVICIO DE ROBOT

// Otros servicios básicos
#include "../include/ServiciosBasicos.h"


using namespace XmlRpc;

// Variables globales para manejo de señal y logger
static std::atomic<bool> g_running{true};
static PALogger* g_logger = nullptr;

// Manejador de señales (Ctrl+C, kill)
void handle_signal(int sig){
    if (g_logger) g_logger->info(std::string("[system] signal recibido — ")+(sig==SIGINT?"SIGINT":"SIGTERM"));
    g_running = false;
}

int main(int argc, char** argv){
    if (argc < 3){
        std::cerr << "Uso: " << argv[0] << " <puerto> <users.csv>\n";
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string usersCsv = argv[2];

    // --- Inicialización Fuera del Try (para acceso en cleanup) ---
    PALogger logger(LogLevel::INFO, true, "servidor.log");
    g_logger = &logger; // Permitir al manejador de señal usar el logger

    // Usamos punteros para controlar el ciclo de vida y la desconexión
    std::shared_ptr<ArduinoService> arduinoService = nullptr;
    RobotService* pRobotService = nullptr; // Puntero crudo para acceso fácil en catch/finally

    try {
        logger.info("=== INICIANDO SERVIDOR XML-RPC ===");
        logger.info("[system] puerto=" + std::to_string(port));
        logger.info("[system] users.csv=" + usersCsv);

        // Señales para apagado prolijo
        std::signal(SIGINT,  handle_signal);        //METIÓ ESTAS DOS LINEAS EN EL TRY
        std::signal(SIGTERM, handle_signal);

        // Crear servidor XML-RPC
        XmlRpcServer server;
        XmlRpc::setVerbosity(0); // 0 = sin logs de la librería XmlRpc++

        // === Dependencias (Herramientas Globales) ===
        SessionManager sessions;
        UsersRepoCsv   repo(usersCsv);
        //AGREGÓ LA SIGUIENTE LÍNEA MEDIO INCESEARIO CREO
        repo.ensureDefaultAdmin(); // Asegura que exista admin/1234 si el CSV está vacío

        //HASTA RESGISTRO DE SERVICIOS ES TODO NUEVO

        // Crear e inicializar las herramientas del robot
        arduinoService = std::make_shared<ArduinoService>("/dev/ttyUSB0", 115200); // <-- USA TU PUERTO CORRECTO
        TrajectoryManager trajectoryManager("data/trayectorias/"); // Crea la carpeta si no existe
        RobotService robotSvc(arduinoService, logger, "data/trayectorias/");
        pRobotService = &robotSvc; // Guardamos puntero para poder desconectar al final

        // Intentar conectar el RobotService al inicio
        logger.info("[RobotService] Intentando conectar al robot...");
        if (robotSvc.conectarRobot(3)) { // Intenta conectar 3 veces
             logger.info("[RobotService] Conexión establecida con el robot.");
             // Podrías descomentar la siguiente línea si quieres que haga homing al iniciar:
             // logger.info("[RobotService] Realizando homing inicial...");
             // robotSvc.homing(); // Llamada directa (no RPC)
        } else {
             logger.warning("[RobotService] No se pudo conectar al robot al inicio. Los servicios de robot podrían fallar.");
        }

        // --- Registro de servicios ---

        // Servicio de prueba básico
        ServicioPrueba servicioPrueba(&server);
        logger.info("[system] ServicioPrueba registrado");

        // Servicios de Autenticación (auth.*)
        auth::AuthLogin  mLogin (&server, sessions, logger, repo);
        auth::AuthMe     mMe    (&server, sessions);
        auth::AuthLogout mLogout(&server, sessions, logger);
        logger.info("[system] auth.* registrados");

        // Servicios de Gestión de Usuarios (user.*)
        userrpc::UserRegister       mUR(&server, sessions, repo, logger);
        userrpc::UserUpdate         mUU(&server, sessions, repo, logger);
        userrpc::UserChangePassword mUC(&server, sessions, repo, logger);
        userrpc::UserList           mUL(&server, sessions, repo, logger);
        logger.info("[system] user.* registrados");

        // Servicios del Robot (robot.*)   //ESTAS DOS LÍNEAS SON NUEVAS ==================================
        robot_service_methods::RobotHomingMethod mRH(&server, sessions, logger, robotSvc); // <-- ¡REGISTRADO!
        logger.info("[system] robot.homing registrado");
        // ... (Aquí irían las instancias de RobotMoveMethod, RobotConnectMethod, etc.) ...

        // --- Enlace, Escucha y Bucle Principal ---
        server.bindAndListen(port);
        server.enableIntrospection(true); // Permite system.listMethods etc.
        logger.info("[system] introspección habilitada");
        logger.info("[system] escuchando… (work loop)");

        while (g_running.load()){
            // work() con timeout corto permite reaccionar a señales (Ctrl+C)
            server.work(0.5); // Espera eventos por 0.5 segundos
        }

        logger.info("[system] apagando servidor…");

    } // Fin del try principal
    catch(const std::exception& e){
        logger.error(std::string("[system] ERROR fatal — ")+e.what());
        // Intentar desconectar el robot incluso si hubo un error fatal
        if (pRobotService && pRobotService->estaConectado()) {              //LÍNEAS NUEVAS=====================
             logger.warning("[RobotService] Desconectando robot debido a error fatal...");
             pRobotService->desconectarRobot();
        }
        return 2; // Código de error
    }
    catch(...){
        logger.error("[system] ERROR fatal — excepción no identificada");
        if (pRobotService && pRobotService->estaConectado()) {              //LÍNEAS NUEVAS=====================
             logger.warning("[RobotService] Desconectando robot debido a error fatal...");
             pRobotService->desconectarRobot();
        }
        return 3; // Código de error
    }
    //LÍNEAS NUEVAS=====================
    // --- Desconexión Limpia ---
    // Se ejecuta si salimos del bucle while (por señal) sin errores fatales
    if (pRobotService && pRobotService->estaConectado()) {
        logger.info("[RobotService] Desconectando robot al finalizar...");
        pRobotService->desconectarRobot();
    }

    logger.info("=== SERVIDOR DETENIDO ===");
    return 0; // Éxito
}