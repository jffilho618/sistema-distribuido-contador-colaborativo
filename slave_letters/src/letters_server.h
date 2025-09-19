#pragma once

#include <string>
#include <atomic>

// Servidor escravo para processamento de letras
class LettersServer {
private:
    int port;
    std::atomic<bool> running;

public:
    explicit LettersServer(int server_port);
    ~LettersServer();

    // Controle do servidor
    bool start();
    void stop();
    bool is_running() const;

    // Processamento específico
    int count_letters(const std::string& text);

private:
    // Métodos auxiliares
    std::string process_letters_request(const std::string& text);
    bool is_letter(char c);
};