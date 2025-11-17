#include "../../include/ServiciosRobot/RobotMotorsMethod.h" // <-- CAMBIO
#include <stdexcept>
#include <string>

namespace robot_service_methods {

// --- Constructor ---
// --- CAMBIO: Nombre de la clase y nombre del servicio RPC ---
RobotMotorsMethod::RobotMotorsMethod(XmlRpc::XmlRpcServer* server,
                                           SessionManager& sm,
                                           PALogger& L,
                                           RobotService& rs,
                                           CommandHistory& ch)
    : XmlRpc::XmlRpcServerMethod("robot.setMotors", server), 
      sessions_(sm),
      logger_(L),
      robotService_(rs), 
      history_(ch) {}

// --- Execute ---
void RobotMotorsMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.setMotors";
    std::string user_for_history = "desconocido";
    std::string details_for_history = "N/A";

    try {
        // 1. Validar Parámetros (token + estado)

        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("estado")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token' y 'estado')"); // <-- CAMBIO
        }
        std::string token = std::string(args["token"]);
        
        if (args["estado"].getType() != XmlRpc::XmlRpcValue::TypeBoolean) { // <-- CAMBIO
             throw XmlRpc::XmlRpcException("BAD_REQUEST: Parámetro 'estado' debe ser booleano"); // <-- CAMBIO
        }
        bool estado = (bool)args["estado"]; 

        details_for_history = estado ? "estado: ON" : "estado: OFF";

        // 2. Validar Sesión y Permisos (Op o Admin)
        
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        user_for_history = session.user; // Guardamos el usuario real

        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de motores (" + (estado ? "ON" : "OFF") + ") por: " + session.user); // <-- CAMBIO

        // 3. Llamar a la Lógica de Negocio (RobotService)
        
        history_.addEntry(session.user, METHOD_NAME, details_for_history, false);
        std::string respuestaRobot;
        if (estado) { // <-- CAMBIO
            respuestaRobot = robotService_.activarMotores();
        } else {
            respuestaRobot = robotService_.desactivarMotores();
        }

        // 4. Procesar Respuesta y Armar Resultado
        if (respuestaRobot.rfind("ERROR:", 0) == 0) {
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". Robot dijo: " + respuestaRobot);
             throw XmlRpc::XmlRpcException(respuestaRobot);
        }

        result["ok"] = true;
        result["msg"] = respuestaRobot;
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user + ". Respuesta: " + respuestaRobot);

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
std::string RobotMotorsMethod::help() {
    // <-- CAMBIO: Actualizar la ayuda -->
    return "robot.setMotors({token:string, estado:bool}) -> {ok:bool, msg:string}\n"
           "Activa (true) o desactiva (false) los motores del robot. Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods