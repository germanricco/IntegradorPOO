#ifndef FILE_H
#define FILE_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <system_error>

/**
 * @brief Enumerador para definir el modo de apertura de un archivo.
 */
enum class FileMode {
    READ,
    WRITE,
    APPEND
};


/**
 * @brief Clase para manejar archivos y persistencia de datos.
 */
class File {
    private:
        std::string filePath;
        std::fstream fileStream;
        bool isOpenFlag;
        FileMode currentMode;

        void ensureOpen(FileMode mode);

    public:
        /**
         * @brief Construye un nuevo objeto File.
         * @param filename Nombre del archivo.
         * @param directory Directorio donde se encuentra el archivo.
         */
        File(const std::string& filename, const std::string& directory = "data/");

        /**
         * @brief Destruye el objeto File.
         */
        ~File();

        // No copiable (seguridad)
        File(const File&) = delete;
        File& operator=(const File&) = delete;

        // ----- GESTION DE ARCHIVOS -----

        /**
         * @brief Abre el archivo en el modo especificado.
         * @param mode Modo de apertura del archivo.
         * @throws std::runtime_error si el archivo no puede ser abierto.
         */
        void open(FileMode mode = FileMode::READ);

        /**
         * @brief Cierra el archivo.
         */
        void close();

        /**
         * @brief Verifica si el archivo esta abierto.
         * @return Verdadero si el archivo esta abierto, falso de lo contrario.
         */
        bool isOpen() const;

        /**
         * @brief Verifica si el archivo existe.
         * @return Verdadero si el archivo existe, falso de lo contrario.
         */
        bool exists() const;

        // ----- OPERACIONES -----

        /**
         * @brief Escribe datos en el archivo.
         * @param data Datos a escribir en el archivo.
         * @throws std::runtime_error si el archivo no esta abierto en modo escritura.
         */
        void write(const std::string& data);

        /**
         * @brief Agrega datos al final del archivo.
         * @param data Datos a agregar al final del archivo.
         * @throws std::runtime_error si el archivo no esta abierto en modo append.
         */
        void append(const std::string& data);

        /**
         * @brief Lee todo el contenido del archivo.
         * @return Contenido del archivo.
         * @throws std::runtime_error si el archivo no esta abierto en modo lectura.
         */
        std::string readAll();

        /**
         * @brief Lee las lineas del archivo.
         * @return Vector de lineas del archivo.
         * @throws std::runtime_error si el archivo no esta abierto en modo lectura.
         */
        std::vector<std::string> readLines();

        // ----- UTILIDADES -----

        std::string getFilePath() const;
        std::string getFilename() const;
        std::string getDirectory() const;
        size_t getSize() const;
        bool isEmpty() const;

        /**
         * @brief Obtiene la fecha de última modificación.
         * @return Tiempo de última modificación.
         */
        //std::filesystem::file_time_type getLastModified() const;
        
        /**
         * @brief Elimina el archivo.
         * @return true si se eliminó, false si no existía.
         */
        //bool remove();

        /**
         * @brief Renombra el archivo.
         * @param newFilename Nuevo nombre del archivo.
         * @param newDirectory Nuevo directorio (opcional).
         */
        //void rename(const std::string& newFilename, const std::string& newDirectory = "");

        /**
         * @brief Obtiene el modo actual del archivo.
         * @return Modo actual.
         */
        //FileMode getCurrentMode() const;
};

#endif // FILE_H