#ifndef ROBOT_RUN_FILE_METHOD_H
#define ROBOT_RUN_FILE_METHOD_H


#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../utils/PALogger.h"
#include "../robot_model/RobotService.h"
#include "../common/AuthZ.h"

namespace robot_service_methods {

/**
 * @brief Método RPC 'robot.runFile'
 * * Carga y ejecuta un archivo de trayectoria .gcode desde el servidor.
 * * Cumple con el requisito de "modo automático".
 * * Requiere token de Operador o Admin.
 */
class RobotRunFileMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_; 

public:
    RobotRunFileMethod(XmlRpc::XmlRpcServer* server,
                       SessionManager& sm,
                       PALogger& L,
                       RobotService& rs);

    /**
     * @brief Ejecución del método.
     * * Parámetros esperados en 'params':
     * - 'token' (string): Token de sesión del usuario.
     * - 'nombre' (string): Nombre del archivo .gcode a ejecutar (ej. "mi_prueba.gcode").
     * * Respuesta en 'result':
     * - 'ok' (bool): true si la ejecución fue exitosa.
     * - 'msg' (string): Mensaje de éxito o descripción del error.
     */
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;

    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_RUN_FILE_METHOD_H