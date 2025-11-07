// --- Includes ---
#include "../../include/ServiciosRobot/RobotStopRecordingMethod.h"
#include <stdexcept> 
#include <string>

namespace robot_service_methods {

// --- Constructor ---
RobotStopRecordingMethod::RobotStopRecordingMethod(XmlRpc::XmlRpcServer* server,
                                                   SessionManager& sm,
                                                   PALogger& L,
                                                   RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.finalizarGrabacion", server), // <-- Nombre del método RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

// --- Execute ---
void RobotStopRecordingMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.finalizarGrabacion";
    try {
        // 1. Validar Parámetros (solo token)
        // -----------------------------------------------------------------
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token')");
        }
        std::string token = std::string(args["token"]);
        // -----------------------------------------------------------------

        // 2. Validar Sesión y Permisos (Op o Admin)
        // (Idéntico a los otros métodos)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud por usuario: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        bool exito = robotService_.finalizarGrabacionTrayectoria();
        
        // 4. Procesar Respuesta y Armar Resultado
        // Esta función (a diferencia de iniciar) casi siempre devuelve 'true'
        //
        if (!exito) {
             // Esto solo pasaría si hubiera un error al cerrar el archivo
             logger_.error(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". Error al cerrar el archivo.");
             throw XmlRpc::XmlRpcException("INTERNAL_ERROR: No se pudo finalizar la grabación.");
        } 

        result["ok"] = true;
        result["msg"] = "Grabación finalizada."; 
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user);

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
std::string RobotStopRecordingMethod::help() {
    return "robot.finalizarGrabacion({token:string}) -> {ok:bool, msg:string}\n"
           "Finaliza la grabación de la trayectoria actual.\n"
           "Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods