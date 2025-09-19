#pragma once

#include <string>
#include <atomic>

// Servidor escravo para processamento de números
class NumbersServer {
private:
    int port;
    std::atomic<bool> running;

public:
    explicit NumbersServer(int server_port);
    ~NumbersServer();

    // Controle do servidor
    bool start();
    void stop();
    bool is_running() const;

    // Processamento específico
    int count_numbers(const std::string& text);

private:
    // Métodos auxiliares
    std::string process_numbers_request(const std::string& text);
    bool is_number(char c);
};