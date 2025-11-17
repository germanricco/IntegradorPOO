#include "../../include/ServiciosRobot/RobotModeMethod.h" // <-- CAMBIO
#include <stdexcept>
#include <string>

namespace robot_service_methods {

// --- Constructor ---
// --- CAMBIO: Nombre de la clase y nombre del servicio RPC ---
RobotModeMethod::RobotModeMethod(XmlRpc::XmlRpcServer* server,
                                       SessionManager& sm,
                                       PALogger& L,
                                       RobotService& rs,
                                       CommandHistory& ch)
    : XmlRpc::XmlRpcServerMethod("robot.setMode", server), // <-- CAMBIO a "robot.setMode"
      sessions_(sm),
      logger_(L),
      robotService_(rs), 
      history_(ch) {}

// --- Execute ---
void RobotModeMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.setMode";
    std::string user_for_history = "desconocido";
    std::string details_for_history = "N/A";

    try {
        // 1. Validar Parámetros (token + mode)
        // -----------------------------------------------------------------
        // <-- INICIO DE CAMBIOS MAYORES -->
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("mode")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token' y 'mode')"); // <-- CAMBIO
        }
        std::string token = std::string(args["token"]);
        std::string modo_str = std::string(args["mode"]); // <-- CAMBIO
        
        details_for_history = "mode: " + modo_str; // Preparamos detalles

        // 2. Validar Sesión y Permisos (Op o Admin)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        user_for_history = session.user; // Guardamos usuario real

        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de cambio de modo (" + modo_str + ") por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        
        RobotService::ModoCoordenadas modo_coord;
        
        if (modo_str == "abs") {
            modo_coord = RobotService::ModoCoordenadas::ABSOLUTO;
        } else if (modo_str == "rel") {
            modo_coord = RobotService::ModoCoordenadas::RELATIVO;
        } else {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: 'mode' debe ser 'abs' o 'rel'"); // <-- CAMBIO
        }

        history_.addEntry(session.user, METHOD_NAME, details_for_history, false);

        std::string respuestaRobot;
        if (robotService_.setModoCoordenadas(modo_coord)) {
            respuestaRobot = "Modo seteado a " + modo_str;
        } else {
            // Esto es un error interno, setModoCoordenadas no debería fallar si se
            // comunica bien, pero por si acaso.
            respuestaRobot = "ERROR: No se pudo setear el modo";
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
        history_.addEntry(user_for_history, METHOD_NAME, details_for_history, true);
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        history_.addEntry(user_for_history, METHOD_NAME, "Error inesperado", true);
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotModeMethod::help() {
    // <-- CAMBIO: Actualizar la ayuda -->
    return "robot.setMode({token:string, mode:string}) -> {ok:bool, msg:string}\n"
           "Setea el modo de coordenadas ('abs' o 'rel'). Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods