#include <iostream>
#include <csignal>
#include <atomic>
#include <cstdlib>
#include "XmlRpc.h"

#include "../include/PALogger.h"
#include "../include/ServiciosBasicos.h"

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

using namespace XmlRpc;

static std::atomic<bool> g_running{true};
static PALogger* g_logger = nullptr;

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

    // Logger: INFO muestra INFO/WARNING/ERROR. Cambia a DEBUG si querés más verboso.
    PALogger logger(LogLevel::INFO, true, "servidor.log");
    g_logger = &logger;

    // Señales para apagado prolijo (Ctrl+C o kill)
    std::signal(SIGINT,  handle_signal);
    std::signal(SIGTERM, handle_signal);

    try {
        logger.info("=== INICIANDO SERVIDOR XML-RPC ===");
        logger.info("[system] puerto=" + std::to_string(port));
        logger.info("[system] users.csv=" + usersCsv);

        XmlRpcServer server;
        // Verbosidad del libxmlrpc (0–5). Déjalo en 0 si no querés ruido del lib.
        XmlRpc::setVerbosity(0);

        // Dependencias
        SessionManager sessions;
        UsersRepoCsv   repo(usersCsv); // crea admin/1234 si no existe

        // Registro de servicios de demo (si los tenés)
        ServicioPrueba servicioPrueba(&server);
        logger.info("[system] ServicioPrueba registrado");

        // auth.*
        auth::AuthLogin  mLogin (&server, sessions, logger, repo);
        auth::AuthMe     mMe    (&server, sessions);
        auth::AuthLogout mLogout(&server, sessions, logger);
        logger.info("[system] auth.* registrados");

        // user.*
        userrpc::UserRegister       mUR(&server, sessions, repo, logger);
        userrpc::UserUpdate         mUU(&server, sessions, repo, logger);
        userrpc::UserChangePassword mUC(&server, sessions, repo, logger);
        userrpc::UserList           mUL(&server, sessions, repo, logger);
        logger.info("[system] user.* registrados");

        // Enlace / listen
        server.bindAndListen(port);
        server.enableIntrospection(true);
        logger.info("[system] introspección habilitada");
        logger.info("[system] escuchando… (work loop)");

        // Bucle principal con chequeo de bandera para apagado limpio
        while (g_running.load()){
            // work() con timeout corto permite reaccionar a señales
            server.work(0.5); // 0.5s
        }

        logger.info("[system] apagando…");
        // Si tu XmlRpcServer requiere cleanup explícito, hacelo aquí.

        logger.info("=== SERVIDOR DETENIDO ===");
        return 0;
    }
    catch(const std::exception& e){
        logger.error(std::string("[system] ERROR fatal — ")+e.what());
        return 2;
    }
    catch(...){
        logger.error("[system] ERROR fatal — excepción no identificada");
        return 3;
    }
}

