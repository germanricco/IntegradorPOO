#ifndef FILE_H
#define FILE_H

#include <string>
#include <fstream>
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
        std::string filename;
        std::string directory;
        std::string filePath;
        std::fstream fileStream;
        bool isOpenFlag;

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
        virtual ~File();

        // No copiable (seguridad)
        File(const File&) = delete;
        File& operator=(const File&) = delete;


        // ----- GESTION DE ARCHIVOS -----

        /**
         * @brief Abre el archivo en el modo especificado.
         * @param mode Modo de apertura del archivo.
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
         */
        void write(const std::string& data);

        /**
         * @brief Agrega datos al final del archivo.
         * @param data Datos a agregar al final del archivo.
         */
        void append(const std::string& data);

        /**
         * @brief Lee todo el contenido del archivo.
         * @return Contenido del archivo.
         */
        std::string readAll();

        /**
         * @brief Lee las lineas del archivo.
         * @return Vector de lineas del archivo.
         */
        std::vector<std::string> readLines();

        // ----- UTILIDADES -----
        
        // Cambiar directorio/archivo que se desea trabajar
        void setFilename(const std::string& filename);
        void setDirectory(const std::string& directory);

        // Getters y setters
        std::string getFilename() const;
        std::string getDirectory() const;
        std::string getFilePath() const;
        std::string getFilename() const;
        std::string getDirectory() const;

        // Otros
        size_t getSize() const;
        bool isEmpty() const;
};

#endif // FILE_H