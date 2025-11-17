#include "../../include/ServiciosRobot/RobotStatusMethod.h"
#include <stdexcept>
#include <string>

namespace robot_service_methods {

// --- Constructor ---
RobotStatusMethod::RobotStatusMethod(XmlRpc::XmlRpcServer* server,
                                           SessionManager& sm,
                                           PALogger& L,
                                           RobotService& rs,
                                           CommandHistory& ch)
    : XmlRpc::XmlRpcServerMethod("robot.getStatus", server), // Nombre RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs), 
      history_(ch) {}

// --- Execute ---
void RobotStatusMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.getStatus";
    std::string user_for_history = "desconocido";
    std::string details_for_history = "N/A";

    try {
        // 1. Validar Parámetros (solo token)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión y Permisos (Op o Admin)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        user_for_history = session.user; // Guardamos usuario real


        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de estado por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        // El método obtenerEstado() devuelve el string del robot (M114)
        std::string respuestaRobot = robotService_.obtenerEstado();
        
        // 4. Procesar Respuesta y Armar Resultado
        if (respuestaRobot.rfind("ERROR:", 0) == 0) {
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". Robot dijo: " + respuestaRobot);
             throw XmlRpc::XmlRpcException(respuestaRobot);
        }

        result["ok"] = true;
        result["status"] = respuestaRobot; // <-- CAMBIO CLAVE: Usamos "status"
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user);

    } catch (const XmlRpc::XmlRpcException& e) {
        throw;
    } catch (const std::runtime_error& e) {
        history_.addEntry(user_for_history, METHOD_NAME, e.what(), true);
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        history_.addEntry(user_for_history, METHOD_NAME, "Error inesperado", true);
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotStatusMethod::help() {
    return "robot.getStatus({token:string}) -> {ok:bool, status:string}\n"
           "Obtiene el estado actual del robot (M114). Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods