#include "../../include/ServiciosRobot/RobotListFilesMethod.h"
#include <stdexcept> 
#include <string>

namespace robot_service_methods {

RobotListFilesMethod::RobotListFilesMethod(XmlRpc::XmlRpcServer* server,
                                       SessionManager& sm,
                                       PALogger& L,
                                       RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.listMyFiles", server), // <-- Nombre RPC
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

void RobotListFilesMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.listMyFiles";
    try {
        // 1. Validar Parámetros (solo token)
        // -----------------------------------------------------------------
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);
        // -----------------------------------------------------------------

        // 2. Validar Sesión y obtener ID y Rol
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        int currentUserId = CurrentUser::get(); //
        std::string currentUserRole = session.privilegio;

        // Validar que tengamos un usuario válido (op o admin)
        if (currentUserRole != "admin" && currentUserRole != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de listado por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        std::vector<std::string> archivos = robotService_.listarTrayectorias(currentUserId, currentUserRole);

        // 4. Procesar Respuesta y Armar Resultado
        XmlRpc::XmlRpcValue fileArray;
        fileArray.setSize(archivos.size());
        for (size_t i = 0; i < archivos.size(); ++i) {
            fileArray[i] = archivos[i];
        }

        result["ok"] = true;
        result["files"] = fileArray; // Devolver el array de nombres

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

std::string RobotListFilesMethod::help() {
    return "robot.listMyFiles({token:string}) -> {ok:bool, files:array}\n"
           "Lista los archivos de trayectoria disponibles.\n"
           "Admin: ve todos. Operador: ve solo los propios.";
}

} // namespace robot_service_methods