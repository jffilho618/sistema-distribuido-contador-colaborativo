#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include "numbers_server.h"
#include "logger.h"

std::atomic<bool> keep_running(true);
NumbersServer* server_instance = nullptr;

void signal_handler(int signal) {
    Logger::info_f("Sinal recebido: %d", signal);
    keep_running = false;
    if (server_instance) {
        server_instance->stop();
    }
}

int main(int argc, char* argv[]) {
    // Configurar logs
    Logger::set_component_name("SLAVE-NUMBERS");
    Logger::set_log_level(LogLevel::DEBUG);
    
    Logger::info("=== INICIANDO ESCRAVO DE NÚMEROS ===");
    
    // Configurar handler de sinais para graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Porta do servidor (pode ser passada como argumento)
    int port = 8082;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
            Logger::info_f("Porta definida via argumento: %d", port);
        } catch (const std::exception& e) {
            Logger::warning_f("Argumento de porta inválido '%s', usando porta padrão %d", 
                             argv[1], port);
        }
    }
    
    try {
        // Criar e iniciar servidor
        NumbersServer server(port);
        server_instance = &server;
        
        Logger::info_f("Tentando iniciar servidor na porta %d", port);
        
        // Iniciar servidor em thread separada para permitir graceful shutdown
        std::thread server_thread([&server]() {
            if (!server.start()) {
                Logger::error("Falha ao iniciar servidor");
                keep_running = false;
            }
        });
        
        // Aguardar um pouco para verificar se o servidor iniciou
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (keep_running) {
            Logger::info("=== ESCRAVO DE NÚMEROS INICIADO COM SUCESSO ===");
            Logger::info_f("Servidor escutando na porta %d", port);
            Logger::info("Pressione Ctrl+C para parar o servidor");
            
            // Loop principal
            while (keep_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        Logger::info("Iniciando shutdown graceful...");
        server.stop();
        
        if (server_thread.joinable()) {
            server_thread.join();
        }
        
        Logger::info("=== ESCRAVO DE NÚMEROS FINALIZADO ===");
        
    } catch (const std::exception& e) {
        Logger::error_f("Erro fatal: %s", e.what());
        return 1;
    }
    
    return 0;
}