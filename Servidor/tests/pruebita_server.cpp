// Servidor/tests/pruebita_server.cpp
#include <cstdlib>
#include <string>
#include <iostream>

#include "services/AuthService.h"
#include "services/AuthBootstrap.h"
#include "XmlRpc.h"

// Logger: firma real -> PALogger(LogLevel, bool toConsole, std::string filename)
#include "PALogger.h"

// DB + repo de usuarios (SQLite)
#include "db/SqliteDb.h"
#include "storage/UsersRepoSqlite.h"

// Sesiones
#include "session/SessionManager.h"

// Auth
#include "auth/AuthLogin.h"
#include "auth/AuthLogout.h"
#include "auth/AuthMe.h"

// Users RPC
#include "user/UserList.h"
#include "user/UserUpdate.h"
#include "user/UserChangePassword.h"
#include "user/UserRegister.h"

// Demo simple
#include "ServiciosBasicos.h"


#include "ArduinoService.h"
#include "RobotService.h"
// Servicios del robot
#include "ServiciosRobot/RobotHomingMethod.h"
#include "ServiciosRobot/RobotMotorsMethod.h"
#include "ServiciosRobot/RobotConnectMethod.h"
#include "ServiciosRobot/RobotDisconnectMethod.h"
#include "ServiciosRobot/RobotGripperMethod.h"
#include "ServiciosRobot/RobotModeMethod.h"
#include "ServiciosRobot/RobotStatusMethod.h"
#include "ServiciosRobot/RobotMoveMethod.h"


using namespace XmlRpc;

int main(int argc, char** argv) {
    int port = 8080;
    if (argc >= 2) {
        int p = std::atoi(argv[1]);
        if (p > 0) port = p;
    }

    const std::string dbPath = "data/db/poo.db";

    // ctor correcto del logger (nivel, a consola, archivo)
    PALogger logger(LogLevel::INFO, true, "servidor.log");
    logger.info("=== INICIANDO SERVIDOR XML-RPC ===");
    logger.info(std::string("[system] puerto=") + std::to_string(port));
    logger.info(std::string("[system] db=") + dbPath);

    try {
        // --- DB y repos ---
        init_auth_layer(dbPath, "palabra_secreta");

        // Opción A: si la clase está en namespace storage
        // storage::UsersRepoSqlite repo(db);

        // Opción B: si no tiene namespace (muy común)
        auto& wiring = auth_wiring();
        IUsersRepo& repo = *wiring.repo;

        AuthService auth(repo, "palabra_secreta");
        
        // 1. Creamos el "Intercomunicador" (Capa de Hardware)
        auto arduinoService = std::make_shared<ArduinoService>("/dev/ttyUSB0", 115200);

        // 2. Creamos el "Jefe de Cocina" (Capa de Lógica)
        auto robotService = std::make_shared<RobotService>(arduinoService, logger);

        // 3. ¡Conectamos el robot!
        logger.info("[system] Intentando conectar con el robot...");
        if (!robotService->conectarRobot()) {
            // Si no está el Arduino, solo avisamos, pero el servidor sigue
            logger.error("[system] FALLÓ LA CONEXIÓN CON EL ROBOT (Arduino no encontrado).");
            // Puedes decidir si lanzar una excepción aquí o no.
            // throw std::runtime_error("No se pudo conectar al robot.");
        } else {
            logger.info("[system] Robot conectado exitosamente.");
        }
                
        // --- Infra RPC + sesiones ---
        XmlRpcServer server;
        XmlRpc::setVerbosity(0);
        SessionManager sessions;

        // --- Servicio demo (firma real: sólo XmlRpcServer*) ---
        ServicioPrueba mPrueba(&server);

        // --- auth.* ---
        auth::AuthLogin  mLogin (&server, sessions, logger, repo);
        auth::AuthLogout mLogout(&server, sessions, logger);

        // AuthMe: tu header indica (XmlRpcServer*, SessionManager&) SIN logger
        auth::AuthMe     mMe    (&server, sessions);

        // --- user.* (firmas: (server*, sessions, repo, logger)) ---
        userrpc::UserList           mUList (&server, sessions, repo, logger);
        userrpc::UserUpdate         mUUpd  (&server, sessions, repo, logger);
        userrpc::UserChangePassword mUChg  (&server, sessions, repo, logger);
        userrpc::UserRegister       mUReg  (&server, sessions, repo, logger);
        
        // (No registramos robot.* aquí para evitar dependencias de hardware)

        // Servicios del robot
        robot_service_methods::RobotHomingMethod mRHoming(&server, sessions, logger, *robotService);
        robot_service_methods::RobotMotorsMethod mRMotors(&server, sessions, logger, *robotService);
        robot_service_methods::RobotModeMethod mRMode(&server, sessions, logger, *robotService);
        robot_service_methods::RobotConnectMethod mRConnect(&server, sessions, logger, *robotService);
        robot_service_methods::RobotDisconnectMethod mRDisconnect(&server, sessions, logger, *robotService);
        robot_service_methods::RobotGripperMethod mRGripper(&server, sessions, logger, *robotService);
        robot_service_methods::RobotStatusMethod mRStatus(&server, sessions, logger, *robotService);
        robot_service_methods::RobotMoveMethod mRMove(&server, sessions, logger, *robotService);

        server.enableIntrospection(true);
        server.bindAndListen(port, 32);
        logger.info("[system] escuchando… (work loop)");

        while (true) server.work(0.5);
    } catch (const std::exception& e) {
        logger.error(std::string("FATAL: ") + e.what());
        return 1;
    } catch (...) {
        logger.error("FATAL: excepción desconocida");
        return 1;
    }
}

