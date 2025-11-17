#ifndef ROBOT_LIST_FILES_METHOD_H
#define ROBOT_LIST_FILES_METHOD_H

#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../utils/PALogger.h"
#include "../robot_model/RobotService.h"
#include "../common/AuthZ.h" // Para la función guardSession
#include "../session/CurrentUser.h" // Para CurrentUser::get()

namespace robot_service_methods {

/**
 * @brief Método RPC 'robot.listMyFiles'
 * * Lista los archivos de trayectoria .gcode disponibles.
 * * Si es Admin, lista todos.
 * * Si es Operador, lista solo los propios (basado en el prefijo ID__).
 */
class RobotListFilesMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_; 

public:
    RobotListFilesMethod(XmlRpc::XmlRpcServer* server,
                         SessionManager& sm,
                         PALogger& L,
                         RobotService& rs);

    /**
     * @brief Ejecución del método.
     * * Parámetros esperados en 'params':
     * - 'token' (string): Token de sesión del usuario.
     * * Respuesta en 'result':
     * - 'ok' (bool): true
     * - 'files' (array): Una lista de strings con los nombres de los archivos.
     */
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;

    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_LIST_FILES_METHOD_H