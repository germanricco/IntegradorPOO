#include "Servidor.h"

// CONSTRUCTOR
Servidor::Servidor(const ServidorConfig& config) 
    : config_(config)
    , logger_(LogLevel::INFO, config_.logEnConsola, config_.archivoLog) {
}

Servidor::~Servidor() {
    finalizar();
}


bool Servidor::inicializar(){
    if (inicializado_) return true;
    
    try {
        logger_.info("===== INICIALIZANDO SERVIDOR =====");
        
        //instancia del historial
        commandHistory_ = std::make_shared<CommandHistory>();
        logger_.info("✅ Historial de Comandos inicializado");

        // 1. Base de datos y autenticación
        if (!inicializarBaseDeDatos()) {
            logger_.warning("Base de datos no disponible - continuando sin funciones de autenticación");
            return false;
        }
        
        // 2. Servicios del robot (si está habilitado)
        if (config_.moduloRobotHabilitado && !inicializarServiciosRobot()) {
            logger_.warning("Robot no disponible - continuando sin funciones de robot");
        }
        
        // 3. Servidor RPC
        if (!inicializarServidorRpc()) {
            return false;
        }
        
        inicializado_ = true;
        logger_.info("✅ Servidor inicializado correctamente");
        return true;
        
    } catch (const std::exception& e) {
        logger_.error("Error en inicialización: " + std::string(e.what()));
        return false;
    }
}

bool Servidor::inicializarBaseDeDatos() {
    try {
        logger_.info("Inicializando base de datos...");

        // Inicializar capa de autenticacion
        sessionManager_ = std::make_shared<SessionManager>();
        init_auth_layer(config_.rutaBaseDatos, config_.salt);
        
        // Obtener el wiring de autenticacion
        auto& wiring = auth_wiring();

        if (!wiring.repo) {
            logger_.error("Error: Repositorio de usuarios no inicializado");
            return false;
        }

        IUsersRepo& repo = *wiring.repo;

        // Crear el servicio de autenticacion
        authService_ = std::make_unique<AuthService>(repo, config_.salt);

        if (!authService_) {
            logger_.error("Error: No se pudo crear el servicio de autenticación");
            return false;
        }

        logger_.info("✅ Base de datos inicializada correctamente");
        return true;
    } catch (const std::exception& e) {
        logger_.error("Error inicializando base de datos: " + std::string(e.what()));
        return false;
    }
}

bool Servidor::inicializarServiciosRobot() {
    try {
        logger_.info("Inicializando servicios del robot...");
        
        arduinoService_ = std::make_shared<ArduinoService>(
            config_.puertoSerial, 
            config_.baudrate
        );
        logger_.info("✅ ArduinoService inicializado correctamente");
        
        robotService_ = std::make_shared<RobotService>(
            arduinoService_, 
            logger_,
            config_.directorioTrayectorias
        );
        logger_.info("✅ RobotService inicializado correctamente");
        
        // Intentar conexión (pero no fallar si no hay robot)
        if (robotService_->conectarRobot(3)) {
            logger_.info("✅ Robot conectado exitosamente");
        } else {
            logger_.warning("⚠️  No se pudo conectar al Robot");
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logger_.error("Error inicializando robot: " + std::string(e.what()));
        return false;
    }
}

bool Servidor::inicializarServidorRpc() {
    try {
        // Crear instancia de servidor
        logger_.info("Inicializando servicios de login y autentificacion...");
        servidorRpc_ = std::make_unique<XmlRpc::XmlRpcServer>();
        XmlRpc::setVerbosity(0);
        
        // Registrar metodos de login y autentificacion
        registrarServiciosLogin();
        
        // Registrar los metodos del robot si es necesario
        if (robotService_) {
            registrarMetodosRobot();
        } else {
            logger_.error("⚠️  RobotService es NULL - metodos de Robot no registrados.");
        }
        
        servidorRpc_->enableIntrospection(true);
        servidorRpc_->bindAndListen(config_.puerto, config_.maxConexiones);
        
        logger_.info("✅ Servidor RPC en puerto " + std::to_string(config_.puerto));
        return true;

    } catch (const std::exception& e) {
        logger_.error("Error en servidor RPC: " + std::string(e.what()));
        return false;
    }
}

// ===== REGISTRO DE METODOS =====

void Servidor::registrarServiciosLogin() {
    if (!sessionManager_) {
        logger_.error("💥 Error sessionManager_ es NULL");
        return;
    }

    auto& wiring = auth_wiring();
    IUsersRepo& repo = *wiring.repo;

    logger_.info("Registrando método AuthLogin...");
    mAuthLogin_ = std::make_unique<auth::AuthLogin>(
        servidorRpc_.get(), *sessionManager_, logger_, repo
    );

    logger_.info("Registrando método AuthLogout...");
    mAuthLogout_ = std::make_unique<auth::AuthLogout>(
        servidorRpc_.get(), *sessionManager_, logger_
    );

    logger_.info("Registrando método AuthMe...");
    mAuthMe_ = std::make_unique<auth::AuthMe>(
        servidorRpc_.get(), *sessionManager_
    );

    logger_.info("Registrando métodos de usuario...");
    mUserList_ = std::make_unique<userrpc::UserList>(
        servidorRpc_.get(), *sessionManager_, repo, logger_
    );
    mUserUpdate_ = std::make_unique<userrpc::UserUpdate>(
        servidorRpc_.get(), *sessionManager_, repo, logger_
    );
    mUserChangePassword_ = std::make_unique<userrpc::UserChangePassword>(
        servidorRpc_.get(), *sessionManager_, repo, logger_
    );
    mUserRegister_ = std::make_unique<userrpc::UserRegister>(
        servidorRpc_.get(), *sessionManager_, repo, logger_
    );

  
    logger_.info("✅ Servicios de Login y Autenticacion registrados");
}

void Servidor::registrarMetodosRobot() {
    // REGISTRAMOS DIRECTAMENTE TUS MÉTODOS EXISTENTES DEL ROBOT
    
    mRobotHoming_ = std::make_unique<robot_service_methods::RobotHomingMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotMotors_ = std::make_unique<robot_service_methods::RobotMotorsMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotMode_ = std::make_unique<robot_service_methods::RobotModeMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotConnect_ = std::make_unique<robot_service_methods::RobotConnectMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotDisconnect_ = std::make_unique<robot_service_methods::RobotDisconnectMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotGripper_ = std::make_unique<robot_service_methods::RobotGripperMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotStatus_ = std::make_unique<robot_service_methods::RobotStatusMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotMove_ = std::make_unique<robot_service_methods::RobotMoveMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_, *commandHistory_
    );
    mRobotGetReport_ = std::make_unique<robot_service_methods::RobotGetReportMethod>(
    servidorRpc_.get(), *sessionManager_, logger_, *commandHistory_
    );
    mRobotStartRecording_ = std::make_unique<robot_service_methods::RobotStartRecordingMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_
    );
    mRobotStopRecording_ = std::make_unique<robot_service_methods::RobotStopRecordingMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_
    );
    mRobotRunFile_ = std::make_unique<robot_service_methods::RobotRunFileMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_
    );
    mRobotUploadFile_ = std::make_unique<robot_service_methods::RobotUploadFileMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_
    );
    mRobotListFiles_ = std::make_unique<robot_service_methods::RobotListFilesMethod>(
        servidorRpc_.get(), *sessionManager_, logger_, *robotService_
    );

    logger_.info("✅ Métodos del robot registrados");
}

void Servidor::ejecutar() {
    if (!inicializado_) {
        logger_.error("Servidor no inicializado");
        return;
    }
    
    ejecutandose_ = true;
    logger_.info("=== SERVIDOR EN EJECUCIÓN ===");
    
    while (ejecutandose_) {
        servidorRpc_->work(0.5);
    }
}

void Servidor::finalizar() {
    if (ejecutandose_) {
        logger_.info("Deteniendo servidor...");
        ejecutandose_ = false;
    }
    limpiarRecursos();
}

// ===== FUNCIONES AUXILIARES =====

bool Servidor::estaEjecutandose() const {
     return ejecutandose_; 
}

const ServidorConfig& Servidor::obtenerConfiguracion() const{
    return config_;
}

void Servidor::limpiarRecursos() {
    if (robotService_) {
        robotService_->desconectarRobot();
    }
    inicializado_ = false;
}
