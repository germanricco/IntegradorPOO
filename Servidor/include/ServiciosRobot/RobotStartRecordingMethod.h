#ifndef ROBOT_START_RECORDING_METHOD_H
#define ROBOT_START_RECORDING_METHOD_H

// --- Includes (Casi idénticos a RobotMoveMethod.h) ---
#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../PALogger.h"
#include "../RobotService.h"
#include "../common/AuthZ.h" // Para la función guardSession

namespace robot_service_methods {

/**
 * @brief Método RPC 'robot.iniciarGrabacion'
 * * Inicia la grabación de una trayectoria en el servidor.
 * Requiere token de Operador o Admin y el nombre de la trayectoria.
 */
class RobotStartRecordingMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_; // El servicio que tiene la lógica real

public:
    /**
     * @brief Constructor.
     * @param server El servidor XmlRpc.
     * @param sm El gestor de sesiones.
     * @param L El logger.
     * @param rs El servicio de robot (que contiene el TrajectoryManager).
     */
    RobotStartRecordingMethod(XmlRpc::XmlRpcServer* server,
                              SessionManager& sm,
                              PALogger& L,
                              RobotService& rs);

    /**
     * @brief Ejecución del método.
     * * Parámetros esperados en 'params':
     * - 'token' (string): Token de sesión del usuario.
     * - 'nombre' (string): Nombre lógico para la trayectoria.
     * * Respuesta en 'result':
     * - 'ok' (bool): true si se inició, false si hubo error.
     * - 'msg' (string): Mensaje de éxito o descripción del error.
     */
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;

    /**
     * @brief Devuelve la ayuda del método.
     */
    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_START_RECORDING_METHOD_H