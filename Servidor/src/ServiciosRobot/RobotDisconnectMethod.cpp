#include "../../include/ServiciosRobot/RobotDisconnectMethod.h"
#include <stdexcept>
#include <string>

namespace robot_service_methods {

// --- Constructor ---
RobotDisconnectMethod::RobotDisconnectMethod(XmlRpc::XmlRpcServer* server,
                                             SessionManager& sm,
                                             PALogger& L,
                                             RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.disconnect", server), // Nombre RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

// --- Execute ---
void RobotDisconnectMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.disconnect";
    try {
        // 1. Validar Parámetros (solo token)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión y Permisos (¡SOLO ADMIN!)
        const SessionView& session = guardAdmin(METHOD_NAME, sessions_, token, logger_);
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de desconexión por Admin: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        // El método desconectarRobot() es 'void', simplemente lo llamamos.
        robotService_.desconectarRobot();
        
        // 4. Armar Resultado
        result["ok"] = true;
        result["msg"] = "Robot desconectado.";
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
std::string RobotDisconnectMethod::help() {
    return "robot.disconnect({token:string}) -> {ok:bool, msg:string}\n"
           "Desconecta el servidor del robot. Requiere token de Administrador.";
}

} // namespace robot_service_methods