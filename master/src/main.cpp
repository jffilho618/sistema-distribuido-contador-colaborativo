#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include "master_server.h"
#include "logger.h"

std::atomic<bool> keep_running(true);
MasterServer* server_instance = nullptr;

void signal_handler(int signal) {
    Logger::info_f("Sinal recebido: %d", signal);
    keep_running = false;
    if (server_instance) {
        server_instance->stop();
    }
}

int main(int argc, char* argv[]) {
    // Configurar logs
    Logger::set_component_name("MASTER");
    Logger::set_log_level(LogLevel::DEBUG);
    
    Logger::info("=== INICIANDO SERVIDOR MESTRE ===");
    
    // Configurar handler de sinais para graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Porta do servidor (pode ser passada como argumento)
    int port = 8080;
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
        // Criar servidor mestre
        MasterServer server(port);
        server_instance = &server;
        
        // Configurar escravos (URLs dos containers Docker)
        server.add_slave("slave-letters", "slave-letters", 8081, "/letras", "letters");
        server.add_slave("slave-numbers", "slave-numbers", 8082, "/numeros", "numbers");
        
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
            Logger::info("=== SERVIDOR MESTRE INICIADO COM SUCESSO ===");
            Logger::info_f("Servidor escutando na porta %d", port);
            Logger::info("Aguardando requisições dos clientes...");
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
        
        Logger::info("=== SERVIDOR MESTRE FINALIZADO ===");
        
    } catch (const std::exception& e) {
        Logger::error_f("Erro fatal: %s", e.what());
        return 1;
    }
    
    return 0;
}