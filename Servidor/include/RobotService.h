#ifndef ROBOTSERVICE_H
#define ROBOTSERVICE_H

#include "ArduinoService.h"
#include "PALogger.h"
#include "TrayectoryManager.h"

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

class RobotService {
    public:
        // Enumeraciones de Estado
        enum class ModoOperacion {
            MANUAL,
            AUTOMATICO
        };

        enum class ModoCoordenadas {
            ABSOLUTO,
            RELATIVO
        };

        enum class ModoEjecucion {
            DETENIDO,
            EJECUTANDO,
            PAUSADO
        };


        // Constructor y destructor
        // utilizo shared_ptr para la conexion con el arduino
        // multiples shared_ptr pueden apuntar al mismo objeto y este se destruye
        // automaticamente cuando el ultimo shared_ptr se elimine

        RobotService(shared_ptr<ArduinoService> arduinoService,
                    PALogger& logger,
                    const string& directorioTrayectorias = "data/trayectorias/");

        //? Porque destructor virtual? 
        virtual ~RobotService() = default;   
        //~RobotService();

        // Eliminar operaciones de copia
        RobotService(const RobotService&) = delete;
        RobotService& operator=(const RobotService&) = delete;

        // Gestion de conexion
        bool conectarRobot(int maxReintentos = 3);
        void desconectarRobot();
        bool estaConectado() const;
        
        // Comando Basicos del Robot
        string homing();
        string mover(double x, double y, double z, double velocidad = 100.0);
        
        // Efector Final
        string activarEfector();
        string desactivarEfector();

        // Control de Motores
        string activarMotores();
        string desactivarMotores();
        
        // Reportes
        std::string obtenerEstado();

        // Gestión de modos
        bool setModoOperacion(ModoOperacion modo);
        bool setModoCoordenadas(ModoCoordenadas modo);
        ModoOperacion getModoOperacion() const;
        ModoCoordenadas getModoCoordenadas() const;
        ModoEjecucion getModoEjecucion() const;
        
    private:
        shared_ptr<ArduinoService> arduinoService_;
        PALogger& logger_;
        string directorioTrayectorias_;
        
        // Estado interno
        ModoOperacion modoOperacion_;
        ModoCoordenadas modoCoordenadas_;
        ModoEjecucion modoEjecucion_;
        
        // Métodos privados de ayuda
        string formatearComandoG1(double x, double y, double z, double vel = 0);
        bool validarPosicion(double x, double y, double z);

        // Procesamiento de respuestas
        string procesarRespuesta(const string& respuestaCompleta);
        void logRespuestaCompleta(const string& respuestaCompleta, const string& comando);

        // Constantes para parsing
        static const string PREFIX_INFO;
        static const string PREFIX_ERROR;
        static const string SUFFIX_OK;
};

#endif // ROBOTSERVICE_H