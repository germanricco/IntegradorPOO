#ifndef ROBOT_STOP_RECORDING_METHOD_H
#define ROBOT_STOP_RECORDING_METHOD_H

// --- Includes (Idénticos) ---
#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../PALogger.h"
#include "../RobotService.h"
#include "../common/AuthZ.h" // Para la función guardSession

namespace robot_service_methods {

/**
 * @brief Método RPC 'robot.finalizarGrabacion'
 * * Detiene la grabación de la trayectoria actual.
 * Requiere token de Operador o Admin.
 */
class RobotStopRecordingMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_;

public:
    /**
     * @brief Constructor.
     */
    RobotStopRecordingMethod(XmlRpc::XmlRpcServer* server,
                             SessionManager& sm,
                             PALogger& L,
                             RobotService& rs);

    /**
     * @brief Ejecución del método.
     * * Parámetros esperados en 'params':
     * - 'token' (string): Token de sesión del usuario.
     * * Respuesta en 'result':
     * - 'ok' (bool): true si se finalizó.
     * - 'msg' (string): Mensaje de éxito.
     */
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;

    /**
     * @brief Devuelve la ayuda del método.
     */
    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_STOP_RECORDING_METHOD_H