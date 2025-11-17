#ifndef ARDUINOSERVICE_H
#define ARDUINOSERVICE_H

#include "hardware/SerialCom.h"
#include <string>
#include <memory>
#include <chrono>

#include <thread>
#include <stdexcept>
#include <iostream>

/**
 * @brief Clase de nivel medio para la comunicacion por puerto serie
 * con Arduino Uno utilizando SerialCom
 */
class ArduinoService {
private:
    void limpiarBuffer();
    bool verificarConexion();

    std::unique_ptr<SerialCom> serialCom;
    bool conectado; // Estado
    // Tiempos configurables para la comunicacion
    std::chrono::milliseconds timeoutEstabilizacion;
    std::chrono::milliseconds timeoutRespuesta;

public:
    // Constructor
    ArduinoService(const std::string& puerto = "/dev/ttyUSB0",
                    int baudrate = 115200);
    
    // Destructor
    ~ArduinoService() = default;

    // Gestión de conexión
    bool conectar(int maxReintentos = 3);
    void desconectar();
    bool estaConectado() const;
    

    // Comunicacion
    std::string enviarComando(const std::string& comando,
                              std::chrono::milliseconds timeoutPersonalizado = std::chrono::milliseconds(0));
    
    // Configuracion
    void setTimeoutEstabilizacion(std::chrono::milliseconds timeout);
    void setTimeoutRespuesta(std::chrono::milliseconds timeout);

    // Getters
    std::string getPuerto() const;
    int getBaudrate() const;
};

#endif // ARDUINOSERVICE_H