#ifndef TRAJECTORYMANAGER_H
#define TRAJECTORYMANAGER_H

#include "File.h"
#include <string>
#include <vector>
#include <memory>

using namespace std;

class TrajectoryManager {
public:
    // Crea el gestor indicando el directorio base donde se guardan .gcode
    explicit TrajectoryManager(const std::string& directorioBase = "data/trayectorias/");

    // Gestión de archivos de trayectorias
    bool existeTrayectoria(const std::string& nombreTrayectoria) const;
    std::vector<std::string> listarTrayectorias() const;
    bool eliminarTrayectoria(const std::string& nombreTrayectoria);

    // Flujo de grabación
    bool iniciarGrabacion(const std::string& nombreTrayectoria);
    bool guardarComando(const std::string& comandoGCode);
    bool finalizarGrabacion();

    // Carga completa (línea por línea) de una trayectoria
    std::vector<std::string> cargarTrayectoria(const std::string& nombreTrayectoria) const;

    // Guarda un archivo de trayectoria completo (para subidas)
    bool guardarTrayectoriaCompleta(const std::string& nombreArchivo, const std::string& contenido);

    // Info de estado
    std::string getDirectorioBase() const { return directorioBase; }
    bool        estaGrabando() const { return grabando; }
    std::string getTrayectoriaActual() const { return trayectoriaActual; }

private:
    std::string directorioBase;
    bool grabando;
    std::string trayectoriaActual;             // nombre de archivo (no ruta)
    std::unique_ptr<File> archivoActual;

    // Helpers
    void        crearDirectorioSiNoExiste() const;
    std::string normalizarNombreArchivo(const std::string& nombreTrayectoria) const;

    // Helpers para convención de nombres
    static std::string slugify(const std::string& s);
    static std::string timestamp();
    std::string buildNombreConvencion(int userId, const std::string& nombreLogico) const;
};

#endif // TRAJECTORYMANAGER_H
