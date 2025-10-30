#include "../../include/ServiciosRobot/RobotConnectMethod.h"
#include <stdexcept>
#include <string>

namespace robot_service_methods {

// --- Constructor ---
RobotConnectMethod::RobotConnectMethod(XmlRpc::XmlRpcServer* server,
                                       SessionManager& sm,
                                       PALogger& L,
                                       RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.connect", server), // Nombre RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

// --- Execute ---
void RobotConnectMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.connect";
    try {
        // 1. Validar Parámetros (solo token)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión y Permisos (¡SOLO ADMIN!)
        // Usamos guardAdmin, que ya loguea y lanza excepción si no es Admin
        const SessionView& session = guardAdmin(METHOD_NAME, sessions_, token, logger_);
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de conexión por Admin: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        // El método conectarRobot() de RobotService devuelve un booleano.
        // Lo llamamos con 3 reintentos como en pruebita_server.
        bool exito = robotService_.conectarRobot(3);
        
        // 4. Procesar Respuesta y Armar Resultado
        if (!exito) {
             // Si falla la conexión, lanzamos una excepción
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló la conexión para " + session.user);
             throw XmlRpc::XmlRpcException("CONNECT_FAILED: No se pudo conectar al robot.");
        }

        result["ok"] = true;
        result["msg"] = "Robot conectado exitosamente.";
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user);

    } catch (const XmlRpc::XmlRpcException& e) {
        throw;
    } catch (const std::runtime_error& e) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotConnectMethod::help() {
    return "robot.connect({token:string}) -> {ok:bool, msg:string}\n"
           "Intenta conectar el servidor al robot. Requiere token de Administrador.";
}

} // namespace robot_service_methods