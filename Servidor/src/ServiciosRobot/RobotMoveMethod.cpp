#include "../../include/ServiciosRobot/RobotMoveMethod.h" // <-- CAMBIO
#include <stdexcept> 
#include <string>

namespace robot_service_methods {

// --- Constructor ---
// --- CAMBIO: Nombre de la clase y nombre del servicio RPC ---
RobotMoveMethod::RobotMoveMethod(XmlRpc::XmlRpcServer* server,
                                     SessionManager& sm,
                                     PALogger& L,
                                     RobotService& rs)
    : XmlRpc::XmlRpcServerMethod("robot.move", server), // <-- CAMBIO
      sessions_(sm),
      logger_(L),
      robotService_(rs) {}

// --- Execute ---
void RobotMoveMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    const char* const METHOD_NAME = "robot.move"; // <-- CAMBIO
    try {
        // 1. Validar Parámetros (token + coordenadas)
        // -----------------------------------------------------------------
        // <-- INICIO DE CAMBIOS MAYORES -->
        XmlRpc::XmlRpcValue args = rpc_norm(params);
        if (!args.hasMember("token") || !args.hasMember("x") || !args.hasMember("y") || !args.hasMember("z")) {
            throw XmlRpc::XmlRpcException("BAD_REQUEST: Faltan parámetros (se requiere 'token', 'x', 'y', 'z')");
        }
        std::string token = std::string(args["token"]);

        // Función auxiliar para extraer números (int o double)
        auto getDouble = [&](const char* key) -> double {
            XmlRpc::XmlRpcValue val = args[key];
            if (val.getType() == XmlRpc::XmlRpcValue::TypeInt) {
                return static_cast<double>((int)val);
            }
            if (val.getType() == XmlRpc::XmlRpcValue::TypeDouble) {
                return (double)val;
            }
            throw XmlRpc::XmlRpcException(std::string("BAD_REQUEST: Parámetro '") + key + "' debe ser numérico");
        };

        // Extraer valores
        double x_val = getDouble("x");
        double y_val = getDouble("y");
        double z_val = getDouble("z");
        double vel_val = 100.0; // Valor default de RobotService

        // Velocidad es opcional
        if (args.hasMember("velocidad")) {
            vel_val = getDouble("velocidad");
        }
        // <-- FIN DE CAMBIOS MAYORES -->
        // -----------------------------------------------------------------

        // 2. Validar Sesión y Permisos (Op o Admin)
        // (Esta lógica es idéntica a Homing)
        const SessionView& session = guardSession(METHOD_NAME, sessions_, token, logger_);
        if (session.privilegio != "admin" && session.privilegio != "op") {
            logger_.warning(std::string("[") + METHOD_NAME + "] FORBIDDEN - Se requiere Op o Admin. Usuario: " + session.user);
            throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes (se requiere Op o Admin)");
        }
        logger_.info(std::string("[") + METHOD_NAME + "] Solicitud de movimiento por usuario: " + session.user);

        // 3. Llamar a la Lógica de Negocio (RobotService)
        // <-- CAMBIO: Llamar a mover() con los parámetros -->
        std::string respuestaRobot = robotService_.mover(x_val, y_val, z_val, vel_val);

        // 4. Procesar Respuesta y Armar Resultado
        // (Esta lógica es idéntica a Homing)
        if (respuestaRobot.rfind("ERROR:", 0) == 0) {
             logger_.warning(std::string("[") + METHOD_NAME + "] Falló para " + session.user + ". Robot dijo: " + respuestaRobot);
             throw XmlRpc::XmlRpcException(respuestaRobot);
        }

        result["ok"] = true;
        result["msg"] = respuestaRobot; 
        logger_.info(std::string("[") + METHOD_NAME + "] Éxito para " + session.user + ". Respuesta: " + respuestaRobot);

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

// --- Help ---
std::string RobotMoveMethod::help() {
    // <-- CAMBIO: Actualizar la ayuda -->
    return "robot.move({token:string, x:double, y:double, z:double, [velocidad:double]}) -> {ok:bool, msg:string}\n"
           "Mueve el robot a la posición (X,Y,Z). Requiere token de Operador o Admin.";
}

} // namespace robot_service_methods