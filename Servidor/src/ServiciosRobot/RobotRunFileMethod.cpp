#include "../../include/ServiciosRobot/RobotRunFileMethod.h"
#include <stdexcept> 
#include <string>

namespace robot_service_methods {

RobotRunFileMethod::RobotRunFileMethod(XmlRpc::XmlRpcServer* server,
                                       SessionManager& sm,
                                       PALogger& L,
                                       RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.runFile", server), // <-- Nombre RPC del cliente
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

void RobotRunFileMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.runFile";
    try {
        // 1. Validar Parámetros (token + nombre)
        // -----------------------------------------------------------------
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("nombre")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token' y 'nombre')");
        }
        std::string token = std::string(args["token"]);
        std::string nombreArchivo = std::string(args["nombre"]);

        if (nombreArchivo.empty()) {
             throw XmlRpc::XmlRpcException("BAD_REQUEST: El parámetro 'nombre' no puede estar vacío.");
        }
        // -----------------------------------------------------------------

        // 2. Validar Sesión y Permisos (Op o Admin)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud para ejecutar '" + nombreArchivo + "' por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        std::string respuesta = robotService_.ejecutarTrayectoria(nombreArchivo);

        // 4. Procesar Respuesta y Armar Resultado
        if (respuesta.rfind("ERROR:", 0) == 0) {
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". Robot dijo: " + respuesta);
             throw XmlRpc::XmlRpcException(respuesta);
        }

        result["ok"] = true;
        result["msg"] = respuesta; 
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user + ". Respuesta: " + respuesta);

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

std::string RobotRunFileMethod::help() {
    return "robot.runFile({token:string, nombre:string}) -> {ok:bool, msg:string}\n"
           "Carga y ejecuta un archivo de trayectoria .gcode desde el servidor.\n"
           "Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods