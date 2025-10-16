#include "File.h"

using namespace std;

File::File(const std::string& filename, const std::string& directory)
    : filename(filename),
      directory(directory),
      filePath(directory + "/" + filename),
      isOpenFlag(false) {

        // Crear directorio si no existe
        if (!directory.empty()){
            error_code ec;
            filesystem::create_directory(directory, ec);
        }
}


File::~File() {
    if (isOpenFlag) {
        close();
    }
}


void File::open(FileMode mode) {
    if (isOpen()) {
        close();
    }
    
    ios_base::openmode openMode = ios::binary;
    
    switch (mode) {
        case FileMode::READ:
            openMode |= ios::in;
            break;
        case FileMode::WRITE:
            openMode |= ios::out | ios::trunc;
            break;
        case FileMode::APPEND:
            openMode |= ios::out | ios::app;
            break;
    }
    
    fileStream.open(filePath, openMode);
    if (!fileStream.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + filePath);
    }
    
    isOpenFlag = true;
}

void File::close() {
    if (fileStream.is_open()) {
        fileStream.close();
    }
    isOpenFlag = false;
}

bool File::isOpen() const {
    return isOpenFlag && fileStream.is_open();
}

bool File::exists() const {
    error_code ec;
    return filesystem::exists(filePath, ec);
}

void File::write(const std::string& content) {
    if (!isOpen()) {
        open(FileMode::WRITE);
    }
    
    fileStream << content;
    fileStream.flush();
    
    if (!fileStream.good()) {
        throw runtime_error("Error al escribir en el archivo: " + filePath);
    }
}

void File::append(const std::string& content) {
    if (!isOpen()) {
        open(FileMode::APPEND);
    }
    
    fileStream << content;
    fileStream.flush();
    
    if (!fileStream.good()) {
        throw runtime_error("Error al añadir al archivo: " + filePath);
    }
}

std::string File::readAll() {
    if (isOpen()) {
        close();
    }

    open(FileMode::READ);

    // Resetear flags de error y ir al inicio
    fileStream.clear();
    fileStream.seekg(0, ios::beg);
    
    // Leer todo el contenido usando un stringstream
    stringstream buffer;
    buffer << fileStream.rdbuf();
    
    if (!fileStream.eof() && !fileStream.good()) {
        throw runtime_error("Error al leer el archivo: " + filePath);
    }
    
    return buffer.str();
}

std::vector<std::string> File::readLines() {
    if (!isOpen()) {
        open(FileMode::READ);
    } else {
        close();
        open(FileMode::READ);
    }
    
    vector<string> lines;
    string line;
    
    // Ir al inicio del archivo y resetear flags
    fileStream.clear();
    fileStream.seekg(0, ios::beg);
    
    while (getline(fileStream, line)) {
        lines.push_back(line);
    }
    
    // Limpiar flags de error si llegamos al final del archivo
    if (fileStream.eof()) {
        fileStream.clear();
    }
    
    return lines;
}

// ----- UTILIDADES -----

void File::setFilename(const std::string& filename) {
    this->filename = filename;
    this->filePath = this->directory + "/" + filename;
}

void File::setDirectory(const std::string& directory) {
    this->directory = directory;
    this->filePath = directory + "/" + this->filename;
}

std::string File::getFilePath() const {
    return filePath;
}

std::string File::getFilename() const {
    filesystem::path pathObj(filePath);
    return pathObj.filename().string();
}

std::string File::getDirectory() const {
    filesystem::path pathObj(filePath);
    return pathObj.parent_path().string();
}

size_t File::getSize() const {
    error_code ec;
    return filesystem::file_size(filePath, ec);
}

bool File::isEmpty() const {
    if (!exists()) {
        return true;
    }
    return getSize() == 0;
}