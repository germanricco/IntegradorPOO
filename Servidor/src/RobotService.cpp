#include "RobotService.h"

using namespace std::chrono_literals;

const std::string RobotService::PREFIX_INFO = "INFO:";
const std::string RobotService::PREFIX_ERROR = "ERROR:";
const std::string RobotService::SUFFIX_OK = "OK";

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
        //activarMotores(); // Activar motores al conectar
        logger_.info("Robot conectado y configurado en modo absoluto");
    }

    return conectado;
}

void RobotService::desconectarRobot() {
    if (arduinoService_) {
        //desactivarMotores();
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


// COMPANDOS DE CONTROL

std::string RobotService::homing(){
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }

    try {
        logger_.info("Ejecutando homing");
        modoEjecucion_ = ModoEjecucion::EJECUTANDO;

        std::string respuestaCompleta = arduinoService_->enviarComando("G28\r\n", 5000ms);
        
        logRespuestaCompleta(respuestaCompleta, "G28");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);

        modoEjecucion_ = ModoEjecucion::DETENIDO;
        logger_.info("Homing completado exitosamente");
        return respuestaCliente;

    } catch (const std::exception& e) {
        modoEjecucion_ = ModoEjecucion::DETENIDO;
        logger_.error("Error en homing: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}

/**
 * @brief Mover el robot a las coordenadas x, y, z
 */
std::string RobotService:: mover(double x, double y, double z, double velocidad) {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    if (!validarPosicion(x, y, z)) {
        return "ERROR: Posición fuera de límites";
    }
    
    try {
        // Parsear a GCode   
        std::string comando = formatearComandoG1(x, y, z, velocidad);
        logger_.info("Ejecutando movimiento");
        modoEjecucion_ = ModoEjecucion::EJECUTANDO;

        // Enviar comando a Firmware y recibir rta.
        std::string respuestaCompleta = arduinoService_->enviarComando(comando, 2000ms);
        
        logRespuestaCompleta(respuestaCompleta, comando);
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);

        modoEjecucion_ = ModoEjecucion::DETENIDO;
        return respuestaCliente;

    } catch (const std::exception& e) {
        modoEjecucion_ = ModoEjecucion::DETENIDO;
        logger_.error("Error en movimiento: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}


std::string RobotService::activarEfector() {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
        logger_.info("Activando efector final");

        std::string respuestaCompleta = arduinoService_->enviarComando("M3\r\n", 5000ms);

        logRespuestaCompleta(respuestaCompleta, "M3");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);

        return respuestaCliente;

    } catch (const std::exception& e) {
        logger_.error("Error activando efector: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}


std::string RobotService::desactivarEfector() {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
        logger_.info("Desactivando efector final");

        std::string respuestaCompleta = arduinoService_->enviarComando("M5\r\n", 5000ms);

        logRespuestaCompleta(respuestaCompleta, "M5");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);

        return respuestaCliente;

    } catch (const std::exception& e) {
        logger_.error("Error desactivando efector: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}

std::string RobotService::activarMotores() {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
        logger_.info("Activando motores");
        
        std::string respuestaCompleta = arduinoService_->enviarComando("M17\r\n", 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, "M17");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
        //! OJO
        logger_.info("Motores activados exitosamente");
        return respuestaCliente;
        
    } catch (const std::exception& e) {
        logger_.error("Error activando motores: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}

std::string RobotService::desactivarMotores() {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
        logger_.info("Desactivando motores");
        
        std::string respuestaCompleta = arduinoService_->enviarComando("M18\r\n", 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, "M18");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
        //! OJO
        logger_.info("Motores desactivados exitosamente");
        return respuestaCliente;
        
    } catch (const std::exception& e) {
        logger_.error("Error desactivando motores: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}


// M114 - Obtener Estado
std::string RobotService::obtenerEstado() {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
        logger_.info("Solicitando estado del robot");
        
        std::string respuestaCompleta = arduinoService_->enviarComando("M114\r\n", 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, "M114");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
        logger_.info("Estado obtenido exitosamente");
        return respuestaCliente;
        
    } catch (const std::exception& e) {
        logger_.error("Error obteniendo estado: " + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}


bool RobotService::setModoCoordenadas(ModoCoordenadas modo) {
    if (!estaConectado()) {
        return false;
    }
    
    try {
        std::string comando = (modo == ModoCoordenadas::ABSOLUTO) ? "G90\r\n" : "G91\r\n";
        std::string nombreModo = (modo == ModoCoordenadas::ABSOLUTO) ? "ABSOLUTO" : "RELATIVO";
        
        logger_.info("Configurando modo de coordenadas: " + nombreModo);
        
        std::string respuestaCompleta = arduinoService_->enviarComando(comando);
        
        logRespuestaCompleta(respuestaCompleta, comando);
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
        modoCoordenadas_ = modo;
        logger_.info("Modo de coordenadas configurado exitosamente: " + nombreModo);
        return true;
        
    } catch (const std::exception& e) {
        logger_.error("Error cambiando modo coordenadas: " + std::string(e.what()));
        return false;
    }
}


bool RobotService::setModoOperacion(ModoOperacion modo) {
    modoOperacion_ = modo;
    std::string modoStr = (modo == ModoOperacion::MANUAL) ? "MANUAL" : "AUTOMATICO";
    logger_.info("Modo operacion cambiado a: " + modoStr);
    return true;
}

RobotService::ModoOperacion RobotService::getModoOperacion() const {
    return modoOperacion_;
}

RobotService::ModoCoordenadas RobotService::getModoCoordenadas() const {
    return modoCoordenadas_;
}

RobotService::ModoEjecucion RobotService::getModoEjecucion() const {
    return modoEjecucion_;
}


// Métodos privados de ayuda
std::string RobotService::formatearComandoG1(double x, double y, double z, double velocidad) {
    std::ostringstream comando;
    comando << "G1";
    
    if (x != 0.0) comando << " X" << std::fixed << std::setprecision(2) << x;
    if (y != 0.0) comando << " Y" << std::fixed << std::setprecision(2) << y;
    if (z != 0.0) comando << " Z" << std::fixed << std::setprecision(2) << z;
    if (velocidad > 0) comando << " F" << std::fixed << std::setprecision(0) << velocidad;
    
    comando << "\r\n";
    return comando.str();
}

bool RobotService::validarPosicion(double x, double y, double z) {
    // Límites de trabajo del robot (ajustar según tu hardware)
    const double X_MAX = 300.0, Y_MAX = 300.0, Z_MAX = 200.0;
    
    if (x < 0 || x > X_MAX || y < 0 || y > Y_MAX || z < 0 || z > Z_MAX) {
        logger_.warning("Posición fuera de límites: X=" + std::to_string(x) + 
                          " Y=" + std::to_string(y) + " Z=" + std::to_string(z));
        return false;
    }
    return true;
}


// ==============================================================================


std::string RobotService::procesarRespuesta(const std::string& respuestaCompleta) {
    // Inicializar variables
    std::istringstream stream(respuestaCompleta);
    std::string linea;
    std::string mensajeParaCliente;
    bool comandoExitoso = false;

    while (std::getline(stream, linea)) {
        // Limpiar espacios y saltos de línea
        linea.erase(0, linea.find_first_not_of(" \t\r\n"));
        linea.erase(linea.find_last_not_of(" \t\r\n") + 1);

        if (linea.empty()) {
            continue;
        }

        // Verificar si es línea OK
        if (linea == SUFFIX_OK) {
            comandoExitoso = true;
            continue;
        }

        // Extraer mensaje de INFO o ERROR para el cliente
        if (linea.find(PREFIX_INFO) == 0) {
            mensajeParaCliente = linea.substr(PREFIX_INFO.length());
            mensajeParaCliente.erase(0, mensajeParaCliente.find_first_not_of(" \t"));
        } else if (linea.find(PREFIX_ERROR) == 0) {
            mensajeParaCliente = linea.substr(PREFIX_ERROR.length());
            mensajeParaCliente.erase(0, mensajeParaCliente.find_first_not_of(" \t"));
            // Si encontramos ERROR, lanzamos excepción
            throw std::runtime_error(mensajeParaCliente);
        }
    }

    if (!comandoExitoso) {
        throw std::runtime_error("No se recibió confirmación OK del Arduino");
    }

    // Si no se encontró mensaje específico, usar uno genérico
    if (mensajeParaCliente.empty()) {
        mensajeParaCliente = "Comando ejecutado correctamente";
    }

    return mensajeParaCliente;
}


void RobotService::logRespuestaCompleta(const std::string& respuestaCompleta, const std::string& comando) {
    std::istringstream stream(respuestaCompleta);
    std::string linea;
    
    logger_.info("Ejecutando comando: " + comando);
    
    while (std::getline(stream, linea)) {
        linea.erase(0, linea.find_first_not_of(" \t\r\n"));
        linea.erase(linea.find_last_not_of(" \t\r\n") + 1);
        
        if (linea.empty()) continue;
        
        if (linea.find(PREFIX_INFO) == 0) {
            std::string mensaje = linea.substr(PREFIX_INFO.length());
            mensaje.erase(0, mensaje.find_first_not_of(" \t"));
            logger_.info("FIRMWARE: " + mensaje);
        } else if (linea.find(PREFIX_ERROR) == 0) {
            std::string mensaje = linea.substr(PREFIX_ERROR.length());
            mensaje.erase(0, mensaje.find_first_not_of(" \t"));
            logger_.error("FIRMWARE: " + mensaje);
        } else if (linea == SUFFIX_OK) {
            logger_.info("FIRMWARE: OK - Comando completado");
        } else {
            logger_.info("FIRMWARE: " + linea);
        }
    }
}