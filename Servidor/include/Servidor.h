#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <cstdlib>
#include <string>
#include <iostream>
#include <memory>

// Configuracion
#include "ServidorConfig.h"

// Authentication
#include "auth/AuthLogin.h"
#include "auth/AuthLogout.h"
#include "auth/AuthMe.h"

#include "services/AuthService.h"
#include "services/AuthBootstrap.h"

// Users RPC
#include "user/UserList.h"
#include "user/UserUpdate.h"
#include "user/UserChangePassword.h"
#include "user/UserRegister.h"

#include "services/AuthService.h"

// DB + repo de usuarios
#include "storage/UsersRepoSqlite.h"
#include "db/SqliteDb.h"

// Servicios del Robot
#include "ServiciosRobot/RobotHomingMethod.h"
#include "ServiciosRobot/RobotMotorsMethod.h"
#include "ServiciosRobot/RobotConnectMethod.h"
#include "ServiciosRobot/RobotDisconnectMethod.h"
#include "ServiciosRobot/RobotGripperMethod.h"
#include "ServiciosRobot/RobotModeMethod.h"
#include "ServiciosRobot/RobotStatusMethod.h"
#include "ServiciosRobot/RobotMoveMethod.h"

// Libreria
#include "XmlRpc.h"

// Módulos
#include "PALogger.h"
#include "session/SessionManager.h"
#include "ArduinoService.h"
#include "RobotService.h"
#include "TrajectoryManager.h"

// Demo Simple
#include "ServiciosBasicos.h"

class Servidor {
    public:
        // Constructor con configuraciones
        explicit Servidor(const ServidorConfig& config);
        // Destructor
        ~Servidor();

        // No copiable
        Servidor(const Servidor&) = delete;
        Servidor& operator=(const Servidor&) = delete;

        // Gestion del ciclo de vida
        bool inicializar();
        void ejecutar();
        void finalizar();

        // Estado del servidor
        bool estaEjecutandose() const;

        //? Chequear
        const ServidorConfig& obtenerConfiguracion() const;

        // Acceso a módulos (para UI u otros)
        std::shared_ptr<RobotService> obtenerRobotService() const;
        std::shared_ptr<SessionManager> obtenerSessionManager() const;

    private:
        ServidorConfig config_;
        PALogger logger_;
        std::unique_ptr<XmlRpc::XmlRpcServer> servidorRpc_;
        
        // Servicios centrales
        std::shared_ptr<SessionManager> sessionManager_;
        std::unique_ptr<AuthService> authService_;
        std::shared_ptr<ArduinoService> arduinoService_;
        std::shared_ptr<TrajectoryManager> trajectoryManager_;
        std::shared_ptr<RobotService> robotService_;
        
        // CAMBIOS GABI:Métodos RPC (se almacenan para mantenerlos vivos)
        std::unique_ptr<auth::AuthLogin>              mAuthLogin_;
        std::unique_ptr<auth::AuthLogout>             mAuthLogout_;
        std::unique_ptr<auth::AuthMe>                 mAuthMe_;

        std::unique_ptr<userrpc::UserList>            mUserList_;
        std::unique_ptr<userrpc::UserUpdate>          mUserUpdate_;
        std::unique_ptr<userrpc::UserChangePassword>  mUserChangePassword_;
        std::unique_ptr<userrpc::UserRegister>        mUserRegister_;

        std::unique_ptr<robot_service_methods::RobotHomingMethod>      mRobotHoming_;
        std::unique_ptr<robot_service_methods::RobotMotorsMethod>      mRobotMotors_;
        std::unique_ptr<robot_service_methods::RobotConnectMethod>     mRobotConnect_;
        std::unique_ptr<robot_service_methods::RobotDisconnectMethod>  mRobotDisconnect_;
        std::unique_ptr<robot_service_methods::RobotGripperMethod>     mRobotGripper_;
        std::unique_ptr<robot_service_methods::RobotModeMethod>        mRobotMode_;
        std::unique_ptr<robot_service_methods::RobotStatusMethod>      mRobotStatus_;
        std::unique_ptr<robot_service_methods::RobotMoveMethod>        mRobotMove_;
        
        //HASTA ACA LLEGAN LOS CAMBIOS
        
        // Estado
        bool ejecutandose_ = false;
        bool inicializado_ = false;
        
        // Métodos de inicialización
        bool inicializarBaseDeDatos();


        /**
         * @brief Inicializar los servicios del robot. Instancia objetos
         */
        bool inicializarServiciosRobot();


        /**
         * @brief Inicializar el servidor XML-RPC registrando los metodos necesarios
         */
        bool inicializarServidorRpc();


        /**
         * @brief Registrar los metodos de login y autenticación en el servidor XML-RPC
         */
        void registrarServiciosLogin();


        /**
         * @brief Registrar los metodos del robot en el servidor XML-RPC
         */
        void registrarMetodosRobot();
        
        // Métodos de limpieza
        void limpiarRecursos();
};
 

#endif // SERVIDOR_H
