#ifndef ADMIN_GET_LOG_REPORT_METHOD_H
#define ADMIN_GET_LOG_REPORT_METHOD_H

#include "../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../utils/PALogger.h"
#include "../utils/AuditLogReader.h" // ¡Necesitamos esto!
#include "../common/AuthZ.h"

namespace admin_service_methods {

/**
 * @brief Método RPC 'admin.getLogReport'
 * * Lee el log de auditoría 'audit.csv' (el log de trabajo del servidor).
 * * Cumple el Requisito #3 del PDF.
 * * Requiere token de Admin.
 */
class AdminGetLogReportMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    AuditLogReader& logReader_; // La herramienta principal de este servicio

public:
    AdminGetLogReportMethod(XmlRpc::XmlRpcServer* server,
                         SessionManager& sm,
                         PALogger& L,
                         AuditLogReader& lr);

    /**
     * @brief Ejecución del método.
     * * Parámetros esperados en 'params':
     * - 'token' (string): Token de sesión del Admin.
     * - 'filter_user' (string, opcional): Filtra por nombre de usuario.
     * - 'filter_response' (string, opcional): Filtra si la respuesta contiene este texto.
     * * Respuesta en 'result':
     * - 'ok' (bool): true
     * - 'log_entries' (array): Una lista de structs con los datos del CSV.
     */
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace admin_service_methods

#endif // ADMIN_GET_LOG_REPORT_METHOD_H