#include "../include/TrajectoryManager.h" // Incluir el header correspondiente
#include <filesystem> // Para manejo de directorios y archivos
#include <iostream>   // Para mensajes de log/error (std::cerr, std::cout)
#include <stdexcept>  // Para lanzar excepciones (std::runtime_error)
#include <fstream>    // Necesario para que File.cpp funcione correctamente

// Alias corto para el namespace filesystem
namespace fs = std::filesystem;

// --- Constructor ---
TrajectoryManager::TrajectoryManager(const std::string& directorioBase)
    : directorioBase(directorioBase), grabando(false), archivoActual(nullptr) {
    // Asegurarse de que el directorio base exista al crear el manager
    try {
        crearDirectorioSiNoExiste();
    } catch (const std::exception& e) {
        // Si el directorio base no se puede crear, es un error crítico.
        // Podríamos loguearlo aquí o simplemente dejar que la excepción suba.
        std::cerr << "ERROR CRÍTICO: No se pudo crear/acceder al directorio base de trayectorias: "
                  << directorioBase << " - " << e.what() << std::endl;
        throw; // Relanzar la excepción para detener la inicialización si es necesario.
    }
}

// --- Gestión de Archivos (Básica) ---

bool TrajectoryManager::existeTrayectoria(const std::string& nombreTrayectoria) const {
    std::string nombreNorm = normalizarNombreArchivo(nombreTrayectoria);
    std::string rutaCompleta = directorioBase + "/" + nombreNorm;
    // Usamos filesystem::exists para verificar si el archivo existe en disco
    std::error_code ec; // Para evitar que exists lance excepción si hay problemas de permisos
    return fs::exists(rutaCompleta, ec);
}

std::vector<std::string> TrajectoryManager::listarTrayectorias() const {
    std::vector<std::string> lista;
    try {
        // Iteramos sobre las entradas del directorio base
        for (const auto& entry : fs::directory_iterator(directorioBase)) {
            // Verificamos si es un archivo regular y tiene la extensión .gcode
            if (entry.is_regular_file() && entry.path().extension() == ".gcode") {
                lista.push_back(entry.path().filename().string()); // Añadimos solo el nombre del archivo
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al listar trayectorias en " << directorioBase << ": " << e.what() << std::endl;
        // Devolvemos lista vacía en caso de error
    }
    return lista;
}

bool TrajectoryManager::eliminarTrayectoria(const std::string& nombreTrayectoria) {
    if (grabando && trayectoriaActual == normalizarNombreArchivo(nombreTrayectoria)) {
        std::cerr << "Error: No se puede eliminar la trayectoria que se está grabando actualmente." << std::endl;
        return false;
    }
    std::string nombreNorm = normalizarNombreArchivo(nombreTrayectoria);
    std::string rutaCompleta = directorioBase + "/" + nombreNorm;
    std::error_code ec;
    bool removido = fs::remove(rutaCompleta, ec);
    if (ec) {
        std::cerr << "Error al eliminar trayectoria " << nombreNorm << ": " << ec.message() << std::endl;
        return false;
    }
    if (!removido && fs::exists(rutaCompleta)) {
         std::cerr << "No se pudo eliminar la trayectoria " << nombreNorm << " (quizás no existía)." << std::endl;
         return false; // El archivo no se borró (podría no existir)
    }
     std::cout << "Trayectoria eliminada: " << nombreNorm << std::endl;
    return true;
}


// --- Guardar Trayectoria ---

bool TrajectoryManager::iniciarGrabacion(const std::string& nombreTrayectoria) {
    if (grabando) {
        // Finalizar implícitamente la grabación anterior si se inicia una nueva?
        // O devolver error? Por ahora, devolvemos error.
        std::cerr << "Advertencia: Ya hay una grabación en curso (" << trayectoriaActual << "). Finalícela primero." << std::endl;
        return false;
    }

    // Normalizamos el nombre y construimos la ruta
    trayectoriaActual = normalizarNombreArchivo(nombreTrayectoria);
    // Nota: La clase File ya añade el directorioBase, así que solo pasamos el nombre.
    // std::string rutaCompleta = directorioBase + "/" + trayectoriaActual;

    try {
        // Creamos una nueva instancia de File. make_unique maneja la memoria.
        // El constructor de File ya maneja la creación de directorios si es necesario.
        archivoActual = std::make_unique<File>(trayectoriaActual, directorioBase);
        // Abrimos en modo WRITE, que trunca (borra) el archivo si ya existe.
        archivoActual->open(FileMode::WRITE);
        grabando = true;
        std::cout << "Iniciando grabación en: " << archivoActual->getFilePath() << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error al iniciar grabación para " << trayectoriaActual << ": " << e.what() << std::endl;
        // Limpiamos el estado en caso de error
        grabando = false;
        trayectoriaActual = "";
        archivoActual = nullptr; // unique_ptr se resetea
        return false;
    }
}

bool TrajectoryManager::guardarComando(const std::string& comandoGCode) {
    if (!grabando) {
        // No estamos grabando, no hacemos nada (o podríamos loguear/lanzar error)
        return false; // Devolver false si no se guardó
    }
    if (!archivoActual) {
        std::cerr << "Error interno: Grabando=true pero archivoActual es null." << std::endl;
        return false;
    }

    try {
        // File::append se encarga de abrir/reabrir en modo APPEND si es necesario
        archivoActual->append(comandoGCode + "\n");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error al guardar comando '" << comandoGCode << "' en " << trayectoriaActual << ": " << e.what() << std::endl;
        // Podríamos detener la grabación aquí si falla la escritura
        // finalizarGrabacion(); // Opcional: Abortar grabación en error
        return false;
    }
}

bool TrajectoryManager::finalizarGrabacion() {
    if (!grabando) {
        // No había grabación activa, no es un error, solo informativo.
        std::cout << "No había ninguna grabación activa para finalizar." << std::endl;
        return true; // Consideramos éxito ya que no había nada que hacer.
    }

    std::cout << "Finalizando grabación de: " << trayectoriaActual << std::endl;
    bool exito = true;
    try {
        if (archivoActual && archivoActual->isOpen()) {
            archivoActual->close(); // Cerramos el archivo explícitamente
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al cerrar el archivo de grabación " << trayectoriaActual << ": " << e.what() << std::endl;
        exito = false; // Marcamos como fallo si el cierre falla
    }

    // Reseteamos el estado independientemente del éxito/fallo del cierre
    grabando = false;
    trayectoriaActual = "";
    archivoActual = nullptr; // Liberamos el unique_ptr

    return exito;
}

// --- Cargar Trayectoria ---

std::vector<std::string> TrajectoryManager::cargarTrayectoria(const std::string& nombreTrayectoria) const {
    std::string nombreNorm = normalizarNombreArchivo(nombreTrayectoria);

    try {
        // Creamos un objeto File solo para esta operación de lectura
        File archivo(nombreNorm, directorioBase);
        if (!archivo.exists()) {
             // Lanzamos excepción si el archivo no se encuentra
             throw std::runtime_error("El archivo de trayectoria no existe: " + nombreNorm);
        }
        // Usamos readLines que ya maneja la apertura/cierre en modo lectura
        return archivo.readLines();
    } catch (const std::exception& e) {
        std::cerr << "Error al cargar trayectoria '" << nombreNorm << "': " << e.what() << std::endl;
        // Devolvemos un vector vacío para señalar el error
        return {};
    }
}

// --- Métodos Privados Auxiliares ---

void TrajectoryManager::crearDirectorioSiNoExiste() {
    // Usamos la librería filesystem para crear directorios
    std::error_code ec;
    fs::create_directories(directorioBase, ec);
    // create_directories no da error si el directorio ya existe
    if (ec) {
        // Si hay otro error (ej. permisos), lanzamos una excepción
        throw std::runtime_error("No se pudo crear el directorio base '" + directorioBase + "': " + ec.message());
    }
}

std::string TrajectoryManager::normalizarNombreArchivo(const std::string& nombreTrayectoria) const {
    // Asegura que termine con .gcode y limpia caracteres básicos
    // (Podría ser más robusto eliminando '/', '\', etc.)
    std::string nombre = nombreTrayectoria;
    std::string extension = ".gcode";
    
    // Si ya termina con .gcode, no lo agregamos de nuevo
    if (nombre.length() >= extension.length() && 
        nombre.substr(nombre.length() - extension.length()) == extension) {
        return nombre;
    }
    
    // Agregar extensión .gcode si no la tiene
    return nombre + extension;
}