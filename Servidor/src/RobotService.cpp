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
        modoEjecucion_ = ModoEjecucion::EJECUTANDO;

        std::string respuestaCompleta = arduinoService_->enviarComando("G28\r\n", 6000ms);
        
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


// G1 - MOVIMIENTO
std::string RobotService::mover(double x, double y, double z, double velocidad) {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
        // Parsear a GCode   
        std::string comando = formatearComandoG1(x, y, z, velocidad);
        modoEjecucion_ = ModoEjecucion::EJECUTANDO;

        // Enviar comando a Firmware y recibir rta.
        std::string respuestaCompleta = arduinoService_->enviarComando(comando, 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, comando);
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);

        modoEjecucion_ = ModoEjecucion::DETENIDO;
        return respuestaCliente;

    } catch (const std::exception& e) {
        modoEjecucion_ = ModoEjecucion::DETENIDO;
        logger_.error("ERROR en mover :" + std::string(e.what()));
        return "ERROR: " + std::string(e.what());
    }
}

std::string RobotService::mover(double x, double y, double z) {
    return mover(x, y, z, 50.0);
}


std::string RobotService::activarEfector() {
    if (!estaConectado()) {
        return "ERROR: Robot no conectado";
    }
    
    try {
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
        std::string respuestaCompleta = arduinoService_->enviarComando("M17\r\n", 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, "M17");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
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
        std::string respuestaCompleta = arduinoService_->enviarComando("M18\r\n", 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, "M18");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
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
        std::string respuestaCompleta = arduinoService_->enviarComando("M114\r\n", 3000ms);
        
        logRespuestaCompleta(respuestaCompleta, "M114");
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
    
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

        std::string respuestaCompleta = arduinoService_->enviarComando(comando);
        
        logRespuestaCompleta(respuestaCompleta, comando);
        std::string respuestaCliente = procesarRespuesta(respuestaCompleta);
        
        modoCoordenadas_ = modo;
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
    
    comando << " X" << std::fixed << std::setprecision(2) << x;
    comando << " Y" << std::fixed << std::setprecision(2) << y;
    comando << " Z" << std::fixed << std::setprecision(2) << z;


    if (velocidad > 0) {
        comando << " F" << std::fixed << std::setprecision(0) << velocidad;
    }
    
    comando << "\r\n";
    return comando.str();
}


// ==============================================================================

std::string RobotService::procesarRespuesta(const std::string& respuestaCompleta) {
    
    // Inicializar variables
    std::istringstream stream(respuestaCompleta);
    std::string linea;
    std::vector<std::string> mensajes;

    bool comandoExitoso = false;
    bool tieneError = false;
    std::string mensajeError;

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

        // Procesar mensajes INFO
        if (linea.find(PREFIX_INFO) == 0) {
            std::string mensaje = linea.substr(PREFIX_INFO.length());
            mensaje.erase(0, mensaje.find_first_not_of(" \t"));
            if (!mensaje.empty()) {
                mensajes.push_back(mensaje);
            }
            continue;
        }

        // Procesar mensajes ERROR
        if (linea.find(PREFIX_ERROR) == 0) {
            std::string mensaje = linea.substr(PREFIX_ERROR.length());
            mensaje.erase(0, mensaje.find_first_not_of(" \t"));
            if (!mensaje.empty()) {
                tieneError = true;
                mensajeError = mensaje;
            }
            continue;
        }

        // Si no tiene prefijo conocido, asumir que es mensaje informativo
        mensajes.push_back(linea);
    }

    // Si hay error, lanzar excepción
    if (tieneError) {
        throw std::runtime_error(mensajeError);
    }

    // Si no se recibió OK, lanzar excepción
    if (!comandoExitoso) {
        throw std::runtime_error("No se recibió confirmación OK del Arduino");
    }

    // Combinar mensajes para el cliente
    if (mensajes.empty()) {
        return "Movimiento completado";
    }

    std::string resultado;
    for (size_t i = 0; i < mensajes.size(); ++i) {
        if (i > 0) resultado += " | ";
        resultado += mensajes[i];
    }

    return resultado;
}


void RobotService::logRespuestaCompleta(const std::string& respuestaCompleta, const std::string& comando) {
    std::istringstream stream(respuestaCompleta);
    std::string linea;
    bool tieneError = false;
    std::vector<std::string> lineasImportantes;
    
    // Filtrar lineas importantes
    while (std::getline(stream, linea)) {
        linea.erase(0, linea.find_first_not_of(" \t\r\n"));
        linea.erase(linea.find_last_not_of(" \t\r\n") + 1);
        
        if (linea.empty()) continue;
        
        if (linea.find(PREFIX_INFO) == 0) {
            lineasImportantes.push_back(linea);

        } else if (linea.find(PREFIX_ERROR) == 0) {
            tieneError = true;
            lineasImportantes.push_back(linea);
        }
    }

    // Loggear de forma condensada
    if (lineasImportantes.empty()) {
        logger_.info("Comando '" + comando + "' - Sin respuesta específica");
        return;
    }

    std::string mensajeLog = "Respuesta '" + comando + "': ";
    for (size_t i = 0; i < lineasImportantes.size(); ++i) {
        if (i > 0) mensajeLog += " | ";
        
        std::string linea = lineasImportantes[i];
        if (linea.find(PREFIX_INFO) == 0) {
            mensajeLog += linea.substr(PREFIX_INFO.length());
        } else if (linea.find(PREFIX_ERROR) == 0) {
            mensajeLog += linea.substr(PREFIX_ERROR.length());
        } else if (linea.find(SUFFIX_OK) == 0) {
            mensajeLog += "OK";
        } else {
            mensajeLog += linea;
        }
    }

    if (tieneError) {
        logger_.error(mensajeLog);
    } else {
        logger_.info(mensajeLog);
    }
}