#include "../../include/ServiciosRobot/RobotGetReportMethod.h"
#include <stdexcept> 
#include <vector> // Necesario para el nuevo filtrado

namespace robot_service_methods {

// --- Constructor (sin cambios) ---
RobotGetReportMethod::RobotGetReportMethod(XmlRpc::XmlRpcServer* server,
                                           SessionManager& sm,
                                           PALogger& L,
                                           CommandHistory& ch)
    : XmlRpc::XmlRpcServerMethod("robot.getReport", server),
      sessions_(sm),
      logger_(L),
      history_(ch) {}

// --- Execute (Modificado para filtros) ---
void RobotGetReportMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.getReport";
    
    try {
        // 1. Validar Parámetros (token + filtros opcionales)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        
        // --- LÓGICA DE FILTROS (Solo para Admin) ---
        std::string filter_user = "";
        bool filter_error_only = false;
        bool filter_success_only = false;

        if (session.privilegio == "admin") {
            // Criterio 1: Filtrar por usuario
            if (args.hasMember("filter_user")) {
                filter_user = std::string(args["filter_user"]);
            }
            // Criterio 2: Filtrar por error (true=solo errores, false=solo éxito)
            if (args.hasMember("filter_error")) {
                if (args["filter_error"].getType() == XmlRpc::XmlRpcValue::TypeBoolean) {
                     if ((bool)args["filter_error"] == true) {
                         filter_error_only = true;
                     } else {
                         filter_success_only = true;
                     }
                }
            }
        }
        // --- FIN LÓGICA DE FILTROS ---


        // 3. Llamar a la Lógica de Negocio (Obtener datos base)
        std::vector<CommandEntry> entries;
        std::string log_msg = "[";
        log_msg += METHOD_NAME;
        log_msg += "] Solicitud de reporte por: " + session.user;

        if (session.privilegio == "admin") {
            log_msg += " (Admin)";
            entries = history_.getAllEntries(); // Admin obtiene todo
        } else {
            log_msg += " (Operador)";
            entries = history_.getEntriesForUser(session.user); // Op obtiene solo lo suyo
        }
        logger_.info(log_msg);

        // 3.5 Aplicar filtros (si es necesario)
        std::vector<CommandEntry> filtered_entries;

        if (filter_user.empty() && !filter_error_only && !filter_success_only) {
            filtered_entries = entries; // Sin filtros, usar la lista completa
        } else {
            // Aplicar filtros
            for (const auto& entry : entries) {
                bool pass_user_filter = true;
                bool pass_error_filter = true;

                // Criterio 1: Usuario
                if (!filter_user.empty()) {
                    pass_user_filter = (entry.username == filter_user);
                }

                // Criterio 2: Error
                if (filter_error_only) {
                    pass_error_filter = entry.was_error;
                } else if (filter_success_only) {
                    pass_error_filter = !entry.was_error;
                }

                if (pass_user_filter && pass_error_filter) {
                    filtered_entries.push_back(entry);
                }
            }
        }
        // --- FIN APLICACIÓN DE FILTROS ---

        
        // 4. Procesar Respuesta (¡Usando la lista filtrada!)
        XmlRpc::XmlRpcValue reportArray;
        reportArray.setSize(filtered_entries.size()); // <-- CAMBIO
        
        int total_comandos = 0;
        int total_errores = 0;

        for (size_t i = 0; i < filtered_entries.size(); ++i) { // <-- CAMBIO
            const auto& entry = filtered_entries[i]; // <-- CAMBIO
            
            XmlRpc::XmlRpcValue entryStruct;
            entryStruct["timestamp"] = entry.timestamp;
            entryStruct["service"] = entry.service_name;
            entryStruct["username"] = entry.username;
            entryStruct["details"] = entry.details;
            entryStruct["error"] = entry.was_error;
            
            reportArray[i] = entryStruct;
            
            total_comandos++;
            if (entry.was_error) {
                total_errores++;
            }
        }

        result["ok"] = true;
        result["total_comandos"] = total_comandos;
        result["total_errores"] = total_errores;
        result["entries"] = reportArray;

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

// --- Help (Actualizado) ---
std::string RobotGetReportMethod::help() {
    return "robot.getReport({token:string, [filter_user:string], [filter_error:bool]}) -> {ok:bool, ...}\n"
           "Devuelve el historial de comandos. \n"
           " - Operador: Devuelve solo su historial.\n"
           " - Admin: Devuelve todo el historial. Puede usar filtros opcionales:\n"
           "     - filter_user (string): Filtra por nombre de usuario.\n"
           "     - filter_error (bool): 'true' para ver solo errores, 'false' para ver solo éxitos.";
}

} // namespace robot_service_methods