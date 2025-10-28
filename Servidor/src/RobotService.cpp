#include "RobotService.h"

using namespace std::chrono_literals;

RobotService::RobotService(shared_ptr<ArduinoService> arduinoService,
                            PALogger& logger,
                            const string& directorioTrayectorias)
    : arduinoService_(arduinoService)
    , logger_(logger)
    , directorioTrayectorias_(directorioTrayectorias)
    , modoOperacion_(ModoOperacion::MANUAL)
    , modoCoordenadas_(ModoCoordenadas::ABSOLUTO)
    , modoEjecucion_(ModoEjecucion::DETENIDO) {

    logger_.info("RobotService Inicializado");
}

RobotService::~RobotService() {
    logger_.info("RobotService Finalizado");
}

bool RobotService::conectarRobot(int maxReintentos) {
    // Verificacion de ardiuno_service
    if (!arduinoService_) {
        logger_.error("ArduinoService no disponible");
        return false;
    }
    // Llamar a metodo conectar
    bool conectado = arduinoService_->conectar(maxReintentos);

    if (conectado) {
        // Configurar estado inicial del robot
        setModoCoordenadas(ModoCoordenadas::ABSOLUTO);
        logger_.info("Robot conectado y configurado en modo absoluto");
    }
    
    return conectado;
}

void RobotService::desconectarRobot() {
    if (arduinoService_) {
        arduinoService_->desconectar();
        modoEjecucion_ = ModoEjecucion::DETENIDO;
        logger_.info("Robot desconectado");
    } else {
        logger_.error("ArduinoService no disponible");
    }
}

bool RobotService::estaConectado() const {
    return arduinoService_ && arduinoService_->estaConectado();
}