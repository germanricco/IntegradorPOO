#include "hardware/SerialCom.h"

//
// ===== CONSTRUCTOR Y DESTRUCTOR =====
//

SerialCom::SerialCom(const std::string& port, int baudrate)
    : port(port),
      baudrate(baudrate),
      fileDescriptor(-1),
      is_connected(false) { 
    // Inicializar parametro originalTTY a cero
    memset(&originalTTY, 0, sizeof(originalTTY));
}

SerialCom::~SerialCom() {
    disconnect();
}

//
// ===== CONEXION Y DESCONEXION =====
//

bool SerialCom::connect() {
    // Si ya esta conectado, no hacer nada
    if (is_connected) {
        std::cerr << "Already connected to " << port << std::endl;
        return true;
    }

    // Abrir el puerto serial en modo lectura/escritura (no bloqueante)
    fileDescriptor = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    // Si no se pudo abrir el puerto, retornar error
    if (fileDescriptor == -1) {
        std::cerr << "Error al abrir puerto serial " << port << ": "
                  << strerror(errno) << std::endl;
        return false;
    }

    // Si no se pudo obtener la configuracion original, cerrar el puerto y retornar error
    if (tcgetattr(fileDescriptor, &originalTTY) != 0) {
        std::cerr << "Error getting original TTY attributes" << std::endl;
        close(fileDescriptor);
        fileDescriptor = -1;
        return false;
    }

    // Si no se pudo configurar el puerto serial, cerrar el puerto y retornar error
    if (!configureSerialPort()) {
        close(fileDescriptor);
        fileDescriptor = -1;
        return false;
    }

    is_connected = true;
    std::cout << "Connected to " << port << " at " << baudrate << " baud." << std::endl;
    return true;
}


void SerialCom::disconnect() {
    if (is_connected) {
        // Restaurar la configuracion original del terminal
        if (tcsetattr(fileDescriptor, TCSANOW, &originalTTY) != 0) {
            std::cerr << "Error restoring TTY attributes" << std::endl;
        }

        // Cerrar el puerto serial
        close(fileDescriptor);
        fileDescriptor = -1;
        is_connected = false;

        std::cout << "Desconectado de: " << port << std::endl;
    }
}


bool SerialCom::isDeviceConnected() const {
    return is_connected;
}

//
// ===== COMUNICACION =====
//

bool SerialCom::sendCommand(const std::string& command) {
    if (!is_connected) {
        std::cerr << "No conectado a un dispositivo." << std::endl;
        return false;
    }

    ssize_t bytes_written = write(fileDescriptor, command.c_str(), command.size());
    
    if (bytes_written < 0) {
        std::cerr << "Error writing to serial port: " 
                  << strerror(errno) << std::endl;
        return false;
    }

    // Esprar hasta que toda la salida haya sido transmitida
    tcdrain(fileDescriptor);
    return bytes_written == static_cast<ssize_t>(command.length());
}

std::string SerialCom::readResponse(int timeoutMs) {
    if (!is_connected) {
        return "";
    }

    std::string response;
    char buffer[256];
    fd_set set;
    struct timeval timeout;

    // Set timeout
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

    // Leer hasta obtener una respuesta completa o timeout
    time_t startTime = time(nullptr);

    while (time(nullptr) - startTime < timeoutMs / 1000) {
        FD_ZERO(&set);
        FD_SET(fileDescriptor, &set);
        
        // select() monitore el fd para datos de lectura con un timeout
        int rv = select(fileDescriptor + 1, &set, NULL, NULL, &timeout);

        if (rv == -1) {
            std::cerr << "Error en select(): " << strerror(errno) << std::endl;
            break;
        } else if (rv == 0) {
            // Timeout occurred
            break;
        } else {
            ssize_t bytesRead = read(fileDescriptor, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response.append(buffer);
                
                // Criterio de mensaje completo: Buscar terminador (ej: 'OK\n' o 'ERROR:...\n')
                // A diferencia de antes, no salimos en el *primer* \n.
                // Damos una pequeña ventana (100ms) para que lleguen más líneas
                // (ej. en M114 que son varias líneas).
                
                // Si encontramos un \n, re-ajustamos el timeout para una espera corta.
                if (response.find('\n') != std::string::npos) {
                    timeout.tv_sec = 0;
                    timeout.tv_usec = 100 * 1000; // 100ms
                }

            } else if (bytesRead == 0) {
                // No data available
                break;
            } else {
                std::cerr << "Error reading from serial port: " << strerror(errno) << std::endl;
                break;
            }
        }
        // Pequeña pausa para no saturar la CPU
        usleep(10000); // 10ms
    }

    return response;
}

//
// ===== GETTERS Y SETTERS =====
//

std::string SerialCom::getPort() const {
    return port;
}

int SerialCom::getBaudrate() const {
    return baudrate;
}

bool SerialCom::setPort(const std::string& newPort) {
    if (is_connected) {
        std::cerr << "No se puede cambiar el puerto mientras esta conectado." << std::endl;
        return false;
    }
    port = newPort;
    return true;
}

bool SerialCom::setBaudrate(int newBaudrate) {
    if (is_connected) {
        std::cerr << "No se puede cambiar el baudrate mientras esta conectado." << std::endl;
        return false;
    }
    // Validacion
    switch(newBaudrate) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
            baudrate = newBaudrate;
            return true;
        default:
            std::cerr << "Baudrate " << newBaudrate << "No soportado." << std::endl;
            return false;
    }
    
    
}


// Metodo privado para configurar el puerto serial
bool SerialCom::configureSerialPort() {
    if (fileDescriptor == -1) {
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    // Obtener la configuración actual
    if (tcgetattr(fileDescriptor, &tty) != 0) {
        return false;
    }

    // Configurar velocidad de baudios
    speed_t speed;
    switch (baudrate) {
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        default:
            return false;
    }
    
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    // Configurar otros parámetros
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8 bits por byte
    tty.c_iflag &= ~IGNBRK;                         // no ignorar break
    tty.c_lflag = 0;                                // no señalización, sin eco, etc.
    tty.c_oflag = 0;                                // sin remapeo, sin delays
    tty.c_cc[VMIN] = 0;                             // lectura no bloqueante
    tty.c_cc[VTIME] = 5;                            // timeout de 0.5 segundos

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // desactivar control de flujo por software
    tty.c_cflag |= (CLOCAL | CREAD);                // ignorar control de modem, habilitar lectura
    tty.c_cflag &= ~(PARENB | PARODD);              // sin paridad
    tty.c_cflag &= ~CSTOPB;                         // 1 bit de parada
    tty.c_cflag &= ~CRTSCTS;                        // desactivar control de flujo por hardware

    // Aplicar configuración
    if (tcsetattr(fileDescriptor, TCSANOW, &tty) != 0) {
        std::cerr << "Error configurando atributos del terminal: " 
                  << strerror(errno) << std::endl;
        return false;
    }

    // Limpiar el buffer del puerto
    tcflush(fileDescriptor, TCIOFLUSH);
    return true;
}