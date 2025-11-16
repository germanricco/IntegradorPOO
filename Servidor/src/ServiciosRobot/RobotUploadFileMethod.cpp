#include "../../include/ServiciosRobot/RobotUploadFileMethod.h"
#include <stdexcept> 
#include <string>

namespace robot_service_methods {

RobotUploadFileMethod::RobotUploadFileMethod(XmlRpc::XmlRpcServer* server,
                                       SessionManager& sm,
                                       PALogger& L,
                                       RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.uploadFile", server), // <-- Nombre RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

void RobotUploadFileMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.uploadFile";
    try {
        // 1. Validar Parámetros (token + nombre + contenido)
        // -----------------------------------------------------------------
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("nombre") || !args.hasMember("contenido")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token', 'nombre' y 'contenido')");
        }
        std::string token = std::string(args["token"]);
        std::string nombreArchivo = std::string(args["nombre"]);
        std::string contenidoArchivo = std::string(args["contenido"]);

        if (nombreArchivo.empty() || contenidoArchivo.empty()) {
             throw XmlRpc::XmlRpcException("BAD_REQUEST: Los parámetros 'nombre' y 'contenido' no pueden estar vacíos.");
        }
        // -----------------------------------------------------------------

        // 2. Validar Sesión y Permisos (Op o Admin)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud para subir '" + nombreArchivo + "' por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        // La función ahora devuelve el nombre final del archivo o "" si falla.
        std::string nombreArchivoFinal = robotService_.guardarTrayectoriaSubida(nombreArchivo, contenidoArchivo);

        // 4. Procesar Respuesta y Armar Resultado
        if (nombreArchivoFinal.empty()) {
             // Falló si la función devolvió un string vacío.
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló el guardado para " + session.user);
             throw XmlRpc::XmlRpcException("ERROR: No se pudo guardar el archivo en el servidor.");
        }

        // ¡Éxito! Devolvemos el nombre real que se guardó.
        result["ok"] = true;
        result["msg"] = "Archivo subido con éxito.";
        result["filename"] = nombreArchivoFinal; // <-- ¡LA NUEVA RESPUESTA!
        
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user + ". Guardado como: " + nombreArchivoFinal);
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

std::string RobotUploadFileMethod::help() {
    return "robot.uploadFile({token:string, nombre:string, contenido:string}) -> {ok:bool, msg:string}\n"
           "Sube un archivo de trayectoria G-Code al servidor.\n"
           "Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods