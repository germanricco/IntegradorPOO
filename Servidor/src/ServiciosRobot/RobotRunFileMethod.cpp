#include "../../include/ServiciosRobot/RobotRunFileMethod.h"
#include <stdexcept> 
#include <string>
#include "../../include/session/CurrentUser.h"

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

        // 2. Validar Sesión y Permisos (Lógica de Admin vs. Operador)
        // -----------------------------------------------------------------
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        
        if (session.privilegio == "admin") {
            // El Admin puede ejecutar cualquier cosa.
            logger_.info(std::string("[") + METHOD_NAME + "] [ADMIN] Solicitad para ejecutar '" + nombreArchivo + "' por: " + session.user);
        
        } else if (session.privilegio == "op") {
            // El Operador solo puede ejecutar sus propios archivos.
            
            // ¡Esta es la forma correcta de obtener el ID del usuario actual!
            int currentUserId = CurrentUser::get(); //

            // El ID no debería ser -1 (default) si la sesión es válida, pero chequear por si acaso.
            if (currentUserId <= 0) {
                 logger_.error(std::string("[") + METHOD_NAME + "] ERROR INTERNO: El usuario '" + session.user + "' tiene un ID inválido (" + std::to_string(currentUserId) + ").");
                 throw XmlRpc::XmlRpcException("INTERNAL_ERROR: No se pudo verificar la propiedad del archivo.");
            }

            std::string prefijoRequerido = std::to_string(currentUserId) + "__";

            if (nombreArchivo.rfind(prefijoRequerido, 0) != 0) {
                // El archivo no empieza con el prefijo del usuario. ¡Denegado!
                logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - El operador '" + session.user + "' (id=" + std::to_string(currentUserId) + ") intentó ejecutar el archivo '" + nombreArchivo + "' (no es de su propiedad).");
                throw XmlRpc::XmlRpcException("FORBIDDEN: Como operador, solo puedes ejecutar archivos de tu propiedad.");
            }
            
            // Si pasó el chequeo, registrar el éxito.
            logger_.info(std::string("[") + METHOD_NAME + "] [OPERATOR] Solicitud para ejecutar '" + nombreArchivo + "' por: " + session.user);

        } else {
            // Ni admin ni "op" (ej. "viewer" o rol desconocido)
            logger_.warning(std::string("[") + METHOD_NAME + "] FORDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }

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