#ifndef ROBOT_UPLOAD_FILE_METHOD_H
#define ROBOT_UPLOAD_FILE_METHOD_H

#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../utils/PALogger.h"
#include "../robot_model/RobotService.h"
#include "../common/AuthZ.h"

namespace robot_service_methods {

/**
 * @brief Método RPC 'robot.uploadFile'
 * * Sube un archivo .gcode completo al servidor.
 * * Cumple con el requisito de "subir/almacenar archivos".
 * * Requiere token de Operador o Admin.
 */
class RobotUploadFileMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_; 

public:
    RobotUploadFileMethod(XmlRpc::XmlRpcServer* server,
                          SessionManager& sm,
                          PALogger& L,
                          RobotService& rs);

    /**
     * @brief Ejecución del método.
     * * Parámetros esperados en 'params':
     * - 'token' (string): Token de sesión del usuario.
     * - 'nombre' (string): Nombre del archivo .gcode a crear (ej. "mi_pieza.gcode").
     * - 'contenido' (string): El contenido G-Code completo del archivo.
     * * Respuesta en 'result':
     * - 'ok' (bool): true si la subida fue exitosa.
     * - 'msg' (string): Mensaje de éxito o descripción del error.
     */
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;

    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_UPLOAD_FILE_METHOD_H