#include "../../include/ServiciosAdmin/AdminGetLogReportMethod.h" // Ajusta la ruta si es necesario
#include <stdexcept> 

namespace admin_service_methods {

// --- Constructor ---
AdminGetLogReportMethod::AdminGetLogReportMethod(XmlRpc::XmlRpcServer* server,
                                           SessionManager& sm,
                                           PALogger& L,
                                           AuditLogReader& lr)
    : XmlRpc::XmlRpcServerMethod("admin.getLogReport", server), // Nombre RPC
      sessions_(sm),
      logger_(L),
      logReader_(lr) {}

// --- Execute ---
void AdminGetLogReportMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "admin.getLogReport";
    
    try {
        // 1. Validar Parámetros (token + filtros opcionales)
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Falta parámetro 'token'");
        }
        std::string token = std::string(args["token"]);

        // 2. Validar Sesión (¡SOLO ADMIN!)
        // Usamos 'guardAdmin' que ya valida el privilegio
        const SessionView& session = guardAdmin(METHOD_NAME, sessions_, token, logger_);
        
        // --- Leer Filtros Opcionales ---
        std::string filter_user = "";
        std::string filter_response = "";

        if (args.hasMember("filter_user")) {
            filter_user = std::string(args["filter_user"]);
        }
        if (args.hasMember("filter_response")) {
            filter_response = std::string(args["filter_response"]);
        }
        
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de reporte CSV por: " + session.user);

        // 3. Llamar a la Lógica de Negocio (AuditLogReader)
        // ¡El lector hace todo el trabajo pesado de I/O y filtrado!
        std::vector<AuditEntry> entries = logReader_.getEntries(filter_user, filter_response);
        
        // 4. Procesar Respuesta y Armar Resultado
        XmlRpc::XmlRpcValue logArray;
        logArray.setSize(entries.size());
        
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            
            XmlRpc::XmlRpcValue entryStruct;
            entryStruct["timestamp"] = entry.timestamp;
            entryStruct["peticion"] = entry.peticion;
            entryStruct["usuario"] = entry.usuario;
            entryStruct["nodo"] = entry.nodo;
            entryStruct["respuesta"] = entry.respuesta;
            
            logArray[i] = entryStruct; // Meter el struct en el array
        }

        result["ok"] = true;
        result["log_entries"] = logArray; // Adjuntar el array de entradas

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
std::string AdminGetLogReportMethod::help() {
    return "admin.getLogReport({token:string, [filter_user:string], [filter_response:string]}) -> {ok:bool, log_entries:[...]}\n"
           "Devuelve el historial del log de auditoría (audit.csv).\n"
           " - filter_user (string): Filtra por nombre de usuario exacto.\n"
           " - filter_response (string): Filtra si la respuesta 'contiene' este texto (ej. 'ERROR' o 'OK').";
}

} // namespace admin_service_methods