#include "../../include/ServiciosRobot/RobotGetReportMethod.h"
#include <stdexcept> 

namespace robot_service_methods {

// --- Constructor ---
RobotGetReportMethod::RobotGetReportMethod(XmlRpc::XmlRpcServer* server,
                                           SessionManager& sm,
                                           PALogger& L,
                                           CommandHistory& ch)
    : XmlRpc::XmlRpcServerMethod("robot.getReport", server), // Nombre RPC
      sessions_(sm),
      logger_(L),
      history_(ch) {}

// --- Execute ---
void RobotGetReportMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.getReport";
    
    try {
        // 1. Validar Parámetros (solo token)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión (Cualquier usuario logueado puede ver su propio reporte)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de reporte por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (CommandHistory)
        std::vector<CommandEntry> entries = history_.getEntriesForUser(session.user);
        
        // 4. Procesar Respuesta y Armar Resultado
        // Convertimos el std::vector<CommandEntry> a un XmlRpcValue::TypeArray
        
        XmlRpc::XmlRpcValue reportArray; // Este será nuestro array
        reportArray.setSize(entries.size());
        
        int total_comandos = 0;
        int total_errores = 0;

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            
            XmlRpc::XmlRpcValue entryStruct; // Este será un struct (diccionario)
            entryStruct["timestamp"] = entry.timestamp;
            entryStruct["service"] = entry.service_name;
            entryStruct["details"] = entry.details;
            entryStruct["error"] = entry.was_error;
            
            reportArray[i] = entryStruct; // Metemos el struct en el array
            
            total_comandos++;
            if (entry.was_error) {
                total_errores++;
            }
        }

        // Armamos el resultado final que verá el cliente
        result["ok"] = true;
        result["total_comandos"] = total_comandos;
        result["total_errores"] = total_errores;
        result["entries"] = reportArray; // Adjuntamos el array de entradas

    } catch (const XmlRpc::XmlRpcException& e) {
        throw; // Relanzamos
    } catch (const std::runtime_error& e) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error de runtime: " + std::string(e.what()));
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: " + std::string(e.what()));
    } catch (...) {
        logger_.error(std::string("[") + METHOD_NAME + "] Error inesperado.");
        throw XmlRpc::XmlRpcException("INTERNAL_ERROR: Ocurrió un error inesperado en " + std::string(METHOD_NAME));
    }
}

// --- Help ---
std::string RobotGetReportMethod::help() {
    return "robot.getReport({token:string}) -> {ok:bool, total_comandos:int, total_errores:int, entries:[...]}\n"
           "Devuelve el historial de comandos para el usuario actual.";
}

} // namespace robot_service_methods