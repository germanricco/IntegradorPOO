// --- Includes ---
#include "../../include/ServiciosRobot/RobotStartRecordingMethod.h"
#include <stdexcept> 
#include <string>

namespace robot_service_methods {

// --- Constructor ---
RobotStartRecordingMethod::RobotStartRecordingMethod(XmlRpc::XmlRpcServer* server,
                                                     SessionManager& sm,
                                                     PALogger& L,
                                                     RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.iniciarGrabacion", server), // <-- Nombre del método RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

// --- Execute ---
void RobotStartRecordingMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.iniciarGrabacion";
    try {
        // 1. Validar Parámetros (token + nombre)
        // -----------------------------------------------------------------
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("nombre")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token' y 'nombre')");
        }
        std::string token = std::string(args["token"]);
        std::string nombreTrayectoria = std::string(args["nombre"]);

        if (nombreTrayectoria.empty()) {
             throw XmlRpc::XmlRpcException("BAD_REQUEST: El parámetro 'nombre' no puede estar vacío.");
        }
        // -----------------------------------------------------------------

        // 2. Validar Sesión y Permisos (Op o Admin)
        // (Esta lógica es idéntica a RobotMoveMethod)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de '" + nombreTrayectoria + "' por usuario: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        bool exito = robotService_.iniciarGrabacionTrayectoria(nombreTrayectoria);

        // 4. Procesar Respuesta y Armar Resultado
        if (!exito) {
             // Esto sucede si ya estaba grabando
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". El robot ya estaba grabando.");
             result["ok"] = false;
             result["msg"] = "ERROR: Ya hay una grabación en curso.";
        } else {
             // Éxito
             result["ok"] = true;
             result["msg"] = "Grabación iniciada para: " + nombreTrayectoria; 
             logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user);
        }

    } catch (const XmlRpc::XmlRpcException& e) {
        throw; // Re-lanzar excepciones RPC
    } catch (const std::runtime_error& e) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotStartRecordingMethod::help() {
    return "robot.iniciarGrabacion({token:string, nombre:string}) -> {ok:bool, msg:string}\n"
           "Inicia el modo 'aprender trayectoria' guardando los comandos en un archivo con el 'nombre' provisto.\n"
           "Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods