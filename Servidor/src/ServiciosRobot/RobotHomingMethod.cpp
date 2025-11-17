#include "../../include/ServiciosRobot/RobotHomingMethod.h" // Incluir nuestro .h
#include <stdexcept> // Para std::runtime_error

namespace robot_service_methods {

// --- Constructor ---
RobotHomingMethod::RobotHomingMethod(XmlRpc::XmlRpcServer* server,
                                     SessionManager& sm,
                                     PALogger& L,
                                     RobotService& rs,
                                     CommandHistory& ch)
    : XmlRpc::XmlRpcServerMethod("robot.homing", server), // Nombre público RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs),
      history_(ch) {}

// --- Execute ---
void RobotHomingMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.homing"; // Para logs y errores
    std::string user_for_history = "desconocido"; //valor por defecto
    try {
        // 1. Validar Parámetros (solo token)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión y Permisos (Op o Admin)
        // Primero, validamos que el token exista y sea válido
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        user_for_history = session.user; //guardamos el nombre de usuario

        // Luego, verificamos el privilegio manualmente
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de homing por usuario: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)

        // Registramos la orden antes de ejecutarla
        history_.addEntry(session.user, METHOD_NAME, "N/A", false);

        std::string respuestaRobot = robotService_.homing();

        // 4. Procesar Respuesta y Armar Resultado
        // Chequeamos si la respuesta de RobotService indica un error
        if (respuestaRobot.rfind("ERROR:", 0) == 0) {
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". Robot dijo: " + respuestaRobot);
             // Lanzamos una excepción que el cliente recibirá como Fault
             throw XmlRpc::XmlRpcException(respuestaRobot);
        }

        result["ok"] = true;
        result["msg"] = respuestaRobot; // El mensaje de éxito que devolvió RobotService
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user + ". Respuesta: " + respuestaRobot);

    } catch (const XmlRpc::XmlRpcException& e) {
        // Errores específicos de XML-RPC (lanzados por nosotros o guardSession)
        // Ya fueron logueados donde se originaron. Solo relanzar.
        history_.addEntry(user_for_history, METHOD_NAME, e.getMessage(), true);
        throw;
    } catch (const std::runtime_error& e) {
        // Otros errores C++ (ej., de RobotService si no pudo conectar)
        history_.addEntry(user_for_history, METHOD_NAME, "Error inesperado", true);
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        // Error inesperado
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotHomingMethod::help() {
    return "robot.homing({token:string}) -> {ok:bool, msg:string}\n"
           "Mueve el robot a la posición de origen (G28). Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods