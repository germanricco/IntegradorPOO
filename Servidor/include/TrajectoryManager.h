#ifndef TRAYECTORYMANAGER_H
#define TRAYECTORYMANAGER_H

#include "File.h"
#include <string>
#include <vector>
#include <memory>

using namespace std;

class TrayectoryManager {
    public:
        // Constructor
        TrayectoryManager(const string& directorioBase = "data/trayectorias/");

        // Gestion de Archivos de trayectorias
        bool existeTrayectoria(const string& nombreTrayectoria) const;
        std::vector<std::string> listarTrayectorias() const;
        bool eliminarTrayectoria(const string& nombreTrayectoria);
        
        // Guardar trayectoria
        bool iniciarGrabacion(const string& nombreTrayectoria);
        bool guardarComando(const string& comandoGCode);
        bool finalizarGrabacion();

        // Cargar trayectoria
        //? Porque retorna un vector y no un queue?
        std::vector<std::string> cargarTrayectoria(const string& nombreTrayectoria) const;

        // Informacion
        std::string getDirectorioBase() const { return directorioBase; }
        bool estaGrabando() const { return grabando; }
        std::string getTrayectoriaActual() const { return trayectoriaActual; }

    private:
        string directorioBase;
        bool grabando;
        string trayectoriaActual;
        unique_ptr<File> archivoActual;

        void crearDirectorioSiNoExiste();
        string normalizarNombreArchivo(const string& nombreTrayectoria) const;
};

#endif // TRAYECTORYMANAGER_H