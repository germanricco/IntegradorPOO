#include "core/Servidor.h"
#include "core/ServidorConfig.h"
#include <iostream>
#include <csignal>

std::shared_ptr<Servidor> servidorGlobal = nullptr;

void manejarSenial(int signal) {
    std::cout << "\n🛑 Recibida señal de interrupción..." << std::endl;
    if (servidorGlobal) {
        servidorGlobal->finalizar();
    }
}

int main(int argc, char** argv) {
    std::signal(SIGINT, manejarSenial);
    std::signal(SIGTERM, manejarSenial);
    
    try {
        // Configuración simple
        ServidorConfig config;
        config.puerto = (argc > 1) ? std::atoi(argv[1]) : 8080;
        
        // Crear e inicializar servidor
        auto servidor = std::make_shared<Servidor>(config);
        servidorGlobal = servidor;
        
        if (!servidor->inicializar()) {
            std::cerr << "❌ No se pudo inicializar el servidor" << std::endl;
            return 1;
        }
        
        std::cout << "🚀 Servidor iniciado en puerto " << config.puerto << std::endl;
        std::cout << "💡 Usa Ctrl+C para detener" << std::endl;
        
        // Ejecutar
        servidor->ejecutar();
        
        std::cout << "👋 Servidor terminado" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Error: " << e.what() << std::endl;
        return 1;
    }
}