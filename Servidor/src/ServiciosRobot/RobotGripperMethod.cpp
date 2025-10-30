#include "../../include/ServiciosRobot/RobotGripperMethod.h" // <-- CAMBIO
#include <stdexcept>
#include <string>

namespace robot_service_methods {

// --- Constructor ---
// --- CAMBIO: Nombre de la clase y nombre del servicio RPC ---
RobotGripperMethod::RobotGripperMethod(XmlRpc::XmlRpcServer* server,
                                             SessionManager& sm,
                                             PALogger& L,
                                             RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.setGripper", server), // <-- CAMBIO a "robot.setGripper"
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

// --- Execute ---
void RobotGripperMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.setGripper"; // <-- CAMBIO
    try {
        // 1. Validar Parámetros (token + estado)
        // -----------------------------------------------------------------
        // <-- INICIO DE CAMBIOS MAYORES -->
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("estado")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token' y 'estado')"); // <-- CAMBIO
        }
        std::string token = std::string(args["token"]);
        
        if (args["estado"].getType() != XmlRpc::XmlRpcValue::TypeBoolean) { // <-- CAMBIO
             throw XmlRpc::XmlRpcException("BAD_REQUEST: Parámetro 'estado' debe ser booleano"); // <-- CAMBIO
        }
        bool estado = (bool)args["estado"]; // <-- CAMBIO (de 'activar' a 'estado')
        // <-- FIN DE CAMBIOS MAYORES -->
        // -----------------------------------------------------------------

        // 2. Validar Sesión y Permisos (Op o Admin)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de gripper (" + (estado ? "ON" : "OFF") + ") por: " + session.user); // <-- CAMBIO

        // 3. Llamar a la Lógica de Negocio (RobotService)
        std::string respuestaRobot;
        if (estado) { // <-- CAMBIO
            respuestaRobot = robotService_.activarEfector();
        } else {
            respuestaRobot = robotService_.desactivarEfector();
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
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotGripperMethod::help() {
    // <-- CAMBIO: Actualizar la ayuda -->
    return "robot.setGripper({token:string, estado:bool}) -> {ok:bool, msg:string}\n"
           "Activa (true) o desactiva (false) el efector final (gripper). Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods