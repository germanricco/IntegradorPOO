#include "ArduinoService.h"

ArduinoService::ArduinoService(const std::string& puerto, int baudrate)
    : serialCom(std::make_unique<SerialCom>(puerto, baudrate)),
      conectado(false),
      timeoutEstabilizacion(3000),
      timeoutRespuesta(2000) {
}

bool ArduinoService::conectar(int maxReintentos) {
    // Verificar si ya esta conectado
    if (conectado) {
        return true;
    }

    // Intentar conectar al Arduino
    for (int intento = 1; intento <= maxReintentos; ++intento) {
        std::cout << "Intentando conexión " << intento << "/" << maxReintentos << std::endl;
        
        if (serialCom->connect()) {
            // Estailizar la conexion y limpiar buffer
            std::this_thread::sleep_for(timeoutEstabilizacion);
            limpiarBuffer();

            conectado = true;
            
            // Testear conexion con comando simple
            if (verificarConexion()) {
                std::cout << "✅ Conexión establecida con Arduino" << std::endl;
                return true;
            }
            
            // Si no responde, desconectamos y reintentamos
            desconectar();
        }
        
        // Esperar 1s antes del siguiente intento
        if (intento < maxReintentos) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    
    std::cout << "❌ No se pudo conectar al Arduino después de " << maxReintentos << " intentos" << std::endl;
    return false;
}

void ArduinoService::desconectar() {
    serialCom->disconnect();
    conectado = false;
    std::cout << "Arduino Desconectado" << std::endl;
}

bool ArduinoService::estaConectado() const {
    return conectado;
}

//! MODIFICAR
bool ArduinoService::verificarConexion() {
    try {
        // Enviar comando de verificación (depende de tu firmware)
        std::string respuesta = enviarComando("M114"); // Obtener POSICION/ESTADO
        return !respuesta.empty() && respuesta.find("error") == std::string::npos;
    } catch (...) {
        return false;
    }
}

// ===== Comunicación =====

std::string ArduinoService::enviarComando(const std::string& comando,
                                               std::chrono::milliseconds timeoutPersonalizado) {
    if (!conectado) {
        throw std::runtime_error("Arduino no conectado");
    }

    if (!serialCom->sendCommand(comando)) {
        throw std::runtime_error("Error enviando comando: " + comando);
    }

    // Pequeña espera para procesamiento del Arduino
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Usamos timeout personalizado si se especifico, si no el default
    int timeoutMs = timeoutPersonalizado.count() > 0 ? 
                   static_cast<int>(timeoutPersonalizado.count()) : 
                   static_cast<int>(timeoutRespuesta.count());

    // Leer Respuesta
    std::string respuesta = serialCom->readResponse(timeoutMs);

    return respuesta;
}

void ArduinoService::limpiarBuffer() {
    if (conectado) {
        // Leer rápidamente cualquier dato residual
        serialCom->readResponse(100);
    }
}

// ===== Getters y Setters =====

std::string ArduinoService::getPuerto() const {
    return serialCom->getPort();
}

int ArduinoService::getBaudrate() const {
    return serialCom->getBaudrate();
}

void ArduinoService::setTimeoutEstabilizacion(std::chrono::milliseconds timeout) {
    timeoutEstabilizacion = timeout;
}

void ArduinoService::setTimeoutRespuesta(std::chrono::milliseconds timeout) {
    timeoutRespuesta = timeout;
}
