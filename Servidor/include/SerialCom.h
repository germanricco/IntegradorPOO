#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <string>

#include <termios.h>    // Manejo de puertos seriales en Linux
#include <fcntl.h>      // open
#include <unistd.h>     // read, write, close
#include <cstring>      // memset
#include <iostream>     // std::cerr

class SerialCom {
    private:
        std::string port;           // Nombre del puerto serial (e.g., "/dev/ttyS0")
        int baudrate;               // Velocidad de comunicacion (e.g., 9600, 115200)
        int fileDescriptor;         // Descriptor del archivo del puerto serial
        bool is_connected;          // Estado de la conexion
        struct termios originalTTY; // Configuracion original del puerto serial

        // Metodo para configurar el puerto serial
        bool configureSerialPort();

    public:
        /**
         * @brief Constructor de SerialCom
         * @param port Nombre del puerto serial (default "/dev/ttyACM0")
         * @param baudrate Velocidad de comunicacion (default 19200)
         */
        SerialCom(const std::string& port = "/dev/ttyUSB0", int baudrate = 19200);

        /**
         * @brief Destructor que asegura la desconexion si esta conectado
         */
        ~SerialCom();

        //
        // ===== CONEXION Y DESCONEXION =====
        //

        /**
         * @brief Intenta conectar al dispositivo en el puerto y baudrate especificados
         * @return true si la conexion fue exitosa, false en caso contrario
         */
        bool connect();

        /**
         * @brief Desconecta del dispositivo si esta conectado
         */
        void disconnect();

        /**
         * @brief Verifica si el dispositivo esta conectado
         * @return true si esta conectado, false en caso contrario
         */
        bool isDeviceConnected() const;

        //
        // ===== COMUNICACION =====
        //
        
        /**
         * @brief Envia un comando al dispositivo
         * @param command Comando a enviar
         * @return true si el comando fue enviado exitosamente, false en caso contrario
         */
        bool sendCommand(const std::string& command);

        /**
         * @brief Lee la respuesta del dispositivo con un timeout
         * @param timeoutMs Tiempo maximo de espera en milisegundos (default 1000ms)
         * @return Respuesta leida del dispositivo, o cadena vacia
         */
        std::string readResponse(int timeoutMs = 2000);

        //
        // ===== GETTERS Y SETTERS =====
        //
        
        /**
         * @brief Obtiene el nombre del puerto serial
         * @return Nombre del puerto serial
         */
        std::string getPort() const;

        /**
         * @brief Obtiene la velocidad de comunicacion
         * @return Velocidad de comunicacion en baudios
         */
        int getBaudrate() const;

        /**
         * @brief Establece un nuevo puerto serial
         * @param newPort Nuevo nombre del puerto serial
         * @return true si el puerto fue cambiado exitosamente, false en caso contrario
         */
        bool setPort(const std::string& newPort);

        /**
         * @brief Establece una nueva velocidad de comunicacion
         * @param baudrate Nueva velocidad en baudios
         * @return true si la velocidad fue cambiada exitosamente, false en caso contrario
         */
        bool setBaudrate(int baudrate);

        // Eliminar operaciones de copia
        SerialCom(const SerialCom&) = delete;
        SerialCom& operator=(const SerialCom&) = delete;
};

#endif