#include "../include/TrajectoryManager.h"
#include "../include/session/CurrentUser.h"  // <-- contexto de usuario (thread_local)
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace fs = std::filesystem;

// ===================== Constructor =====================

TrajectoryManager::TrajectoryManager(const std::string& directorioBase)
    : directorioBase(directorioBase), grabando(false), archivoActual(nullptr) {
    try {
        crearDirectorioSiNoExiste();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: No se pudo crear/acceder al directorio de trayectorias: "
                  << directorioBase << " - " << e.what() << std::endl;
        throw;
    }
}

// ===================== Helpers de convención =====================

std::string TrajectoryManager::slugify(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalnum(uc)) out.push_back((char)std::tolower(uc));
        else if (c==' ' || c=='-' || c=='_') out.push_back('_');
        // otros caracteres se descartan
    }
    if (out.empty()) out = "traj";
    return out;
}

std::string TrajectoryManager::timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

std::string TrajectoryManager::buildNombreConvencion(int userId, const std::string& nombreLogico) const {
    // <userId>__<slug(nombre)>__YYYYMMDD_HHMMSS.gcode
    return std::to_string(userId) + "__" + slugify(nombreLogico) + "__" + timestamp() + ".gcode";
}

// ===================== Gestión de Archivos =====================

bool TrajectoryManager::existeTrayectoria(const std::string& nombreTrayectoria) const {
    std::string nombreNorm = normalizarNombreArchivo(nombreTrayectoria);
    std::string rutaCompleta = directorioBase + "/" + nombreNorm;
    std::error_code ec;
    return fs::exists(rutaCompleta, ec);
}

std::vector<std::string> TrajectoryManager::listarTrayectorias() const {
    std::vector<std::string> lista;
    const int uid = CurrentUser::get(); // -1 si no hay contexto

    try {
        for (const auto& entry : fs::directory_iterator(directorioBase)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".gcode") continue;

            const std::string fname = entry.path().filename().string();

            if (uid >= 0) {
                // Con contexto: solo las del usuario actual (prefijo "<id>__")
                const std::string pref = std::to_string(uid) + "__";
                if (fname.rfind(pref, 0) == 0) lista.push_back(fname);
            } else {
                // Sin contexto: todas (útil para admin/scripts/legacy)
                lista.push_back(fname);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al listar trayectorias en " << directorioBase << ": " << e.what() << std::endl;
    }
    return lista;
}

bool TrajectoryManager::eliminarTrayectoria(const std::string& nombreTrayectoria) {
    // No permitir borrar lo que se está grabando
    if (grabando && trayectoriaActual == normalizarNombreArchivo(nombreTrayectoria)) {
        std::cerr << "Error: no se puede eliminar la trayectoria en grabación." << std::endl;
        return false;
    }

    std::string nombreNorm = normalizarNombreArchivo(nombreTrayectoria);
    std::string rutaCompleta = directorioBase + "/" + nombreNorm;

    std::error_code ec;
    bool removed = fs::remove(rutaCompleta, ec);
    if (ec) {
        std::cerr << "Error al eliminar '" << nombreNorm << "': " << ec.message() << std::endl;
        return false;
    }
    if (!removed && fs::exists(rutaCompleta)) {
        std::cerr << "No se pudo eliminar '" << nombreNorm << "' (¿no existía?)." << std::endl;
        return false;
    }
    std::cout << "Trayectoria eliminada: " << nombreNorm << std::endl;
    return true;
}

// ===================== Flujo de grabación =====================

bool TrajectoryManager::iniciarGrabacion(const std::string& nombreTrayectoria) {
    if (grabando) {
        std::cerr << "Advertencia: ya hay una grabación en curso (" << trayectoriaActual << ")." << std::endl;
        return false;
    }

    trayectoriaActual = normalizarNombreArchivo(nombreTrayectoria);

    try {
        archivoActual = std::make_unique<File>(trayectoriaActual, directorioBase);
        archivoActual->open(FileMode::WRITE);  // trunca si existe
        grabando = true;
        std::cout << "Iniciando grabación en: " << archivoActual->getFilePath() << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error al iniciar grabación para " << trayectoriaActual << ": " << e.what() << std::endl;
        grabando = false;
        trayectoriaActual.clear();
        archivoActual = nullptr;
        return false;
    }
}

bool TrajectoryManager::guardarComando(const std::string& comandoGCode) {
    if (!grabando) return false;
    if (!archivoActual) {
        std::cerr << "Error interno: archivoActual == nullptr con grabación activa." << std::endl;
        return false;
    }

    try {
        archivoActual->append(comandoGCode + "\n");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error al guardar comando en " << trayectoriaActual << ": " << e.what() << std::endl;
        return false;
    }
}

bool TrajectoryManager::finalizarGrabacion() {
    if (!grabando) {
        std::cout << "No había grabación activa para finalizar." << std::endl;
        return true;
    }

    bool ok = true;
    try {
        if (archivoActual && archivoActual->isOpen()) {
            archivoActual->close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al cerrar archivo " << trayectoriaActual << ": " << e.what() << std::endl;
        ok = false;
    }

    grabando = false;
    trayectoriaActual.clear();
    archivoActual = nullptr;
    return ok;
}

// ===================== Carga =====================

std::vector<std::string> TrajectoryManager::cargarTrayectoria(const std::string& nombreTrayectoria) const {
    std::string nombreNorm = normalizarNombreArchivo(nombreTrayectoria);
    try {
        File archivo(nombreNorm, directorioBase);
        if (!archivo.exists()) {
            throw std::runtime_error("El archivo de trayectoria no existe: " + nombreNorm);
        }
        return archivo.readLines();
    } catch (const std::exception& e) {
        std::cerr << "Error al cargar trayectoria '" << nombreNorm << "': " << e.what() << std::endl;
        return {};
    }
}

// ===================== Privados =====================

void TrajectoryManager::crearDirectorioSiNoExiste() const {
    std::error_code ec;
    fs::create_directories(directorioBase, ec);
    if (ec) {
        throw std::runtime_error("No se pudo crear el directorio base '" + directorioBase +
                                 "': " + ec.message());
    }
}

std::string TrajectoryManager::normalizarNombreArchivo(const std::string& nombreTrayectoria) const {
    // Si viene un nombre ya "final" con .gcode → no tocar (compatibilidad con filenames listados)
    if (nombreTrayectoria.size() >= 6) {
        const std::string ext = nombreTrayectoria.substr(nombreTrayectoria.size() - 6);
        if (ext == ".gcode") {
            return nombreTrayectoria;
        }
    }

    // Intentar aplicar convención si hay usuario en contexto
    int uid = CurrentUser::get(); // -1 si no hay contexto
    if (uid >= 0) {
        return buildNombreConvencion(uid, nombreTrayectoria);
    }

    // Sin contexto → legacy: solo agrega ".gcode"
    return nombreTrayectoria + ".gcode";
}
