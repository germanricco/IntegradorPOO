#ifndef ROBOTSERVICE_H
#define ROBOTSERVICE_H

#include "ArduinoService.h"
#include "PALogger.h"
#include "TrajectoryManager.h"

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
        string mover(double x, double y, double z, double velocidad);
        string mover(double x, double y, double z);
        
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
        
        // Gestión de trayectorias
        bool iniciarGrabacionTrayectoria(const std::string& nombreLogico);
        bool finalizarGrabacionTrayectoria();
        bool estaGrabando() const;        
        string ejecutarTrayectoria(const std::string& nombreArchivo);
        std::string guardarTrayectoriaSubida(const std::string& nombreArchivo, const std::string& contenido);

        // Nuevo método para listar archivos
        std::vector<std::string> listarTrayectorias(int userId, const std::string& userRole);

    private:
        shared_ptr<ArduinoService> arduinoService_;
        PALogger& logger_;
        string directorioTrayectorias_;
        std::unique_ptr<TrajectoryManager> trajectoryManager_;
        
        // Estado interno
        ModoOperacion modoOperacion_;
        ModoCoordenadas modoCoordenadas_;
        ModoEjecucion modoEjecucion_;

        bool motoresActivados_ = false;
        
        // Métodos privados de ayuda
        string formatearComandoG1(double x, double y, double z, double vel = 1);
        // Procesamiento de respuestas
        string procesarRespuesta(const string& respuestaCompleta);
        void logRespuestaCompleta(const string& respuestaCompleta, const string& comando);

        // Constantes para parsing
        static const string PREFIX_INFO;
        static const string PREFIX_ERROR;
        static const string SUFFIX_OK;

        std::chrono::milliseconds getTimeoutParaComando(const std::string& lineaGCode) const;

};

#endif // ROBOTSERVICE_H