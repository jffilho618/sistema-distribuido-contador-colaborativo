#include "letters_server.h"
#include "logger.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <cctype>
#include <chrono>

using json = nlohmann::json;

LettersServer::LettersServer(int server_port) : port(server_port), running(false) {
    Logger::info_f("Servidor de letras criado na porta %d", port);
}

LettersServer::~LettersServer() {
    stop();
}

bool LettersServer::start() {
    if (running.load()) {
        Logger::warning("Servidor de letras já está rodando");
        return false;
    }

    try {
        httplib::Server server;

        // Configurar timeout
        server.set_read_timeout(30, 0);
        server.set_write_timeout(30, 0);

        // Health check
        server.Get("/health", [this](const httplib::Request&, httplib::Response& res) {
            json response;
            response["status"] = "healthy";
            response["service"] = "slave-letters";
            response["port"] = port;

            res.set_content(response.dump(), "application/json");
            Logger::debug("Health check requisitado no servidor de letras");
        });

        // Endpoint principal para contar letras
        server.Post("/letras", [this](const httplib::Request& req, httplib::Response& res) {
            Logger::info_f("Servidor mestre conectado ao escravo de letras de %s", req.remote_addr.c_str());
            Logger::info("Requisição de contagem de letras recebida");

            try {
                json request_json = json::parse(req.body);
                std::string text = request_json["text"];

                Logger::debug_f("Processando texto de %zu caracteres para letras", text.length());

                auto start_time = std::chrono::high_resolution_clock::now();

                std::string result = process_letters_request(text);

                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                // Adicionar tempo de processamento
                json result_json = json::parse(result);
                result_json["processing_time_ms"] = duration.count();

                res.set_content(result_json.dump(), "application/json");
                Logger::debug_f("Contagem de letras concluída em %ld ms", duration.count());

            } catch (const std::exception& e) {
                Logger::error_f("Erro na contagem de letras: %s", e.what());

                json error_response;
                error_response["success"] = false;
                error_response["error"] = e.what();
                error_response["count"] = 0;

                res.status = 400;
                res.set_content(error_response.dump(), "application/json");
            }
        });

        // CORS headers
        server.set_post_routing_handler([](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
        });

        Logger::info_f("Iniciando servidor de letras na porta %d", port);
        running = true;

        Logger::info("Rotas configuradas no servidor de letras");

        // Iniciar servidor (bloqueia thread)
        bool started = server.listen("0.0.0.0", port);

        if (!started) {
            Logger::error("Falha ao iniciar servidor HTTP de letras");
            running = false;
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error_f("Erro ao iniciar servidor de letras: %s", e.what());
        running = false;
        return false;
    }
}

void LettersServer::stop() {
    if (running.load()) {
        Logger::info("Parando servidor de letras");
        running = false;
    }
}

bool LettersServer::is_running() const {
    return running.load();
}

std::string LettersServer::process_letters_request(const std::string& text) {
    json result;

    try {
        int letter_count = count_letters(text);

        result["success"] = true;
        result["count"] = letter_count;
        result["service"] = "letters";
        result["processed_characters"] = text.length();

        Logger::info_f("Contagem de letras concluída: %d letras em %zu caracteres",
                      letter_count, text.length());

    } catch (const std::exception& e) {
        result["success"] = false;
        result["error"] = e.what();
        result["count"] = 0;

        Logger::error_f("Erro no processamento de letras: %s", e.what());
    }

    return result.dump();
}

int LettersServer::count_letters(const std::string& text) {
    int count = 0;

    for (char c : text) {
        if (is_letter(c)) {
            count++;
        }
    }

    Logger::debug_f("Contadas %d letras no texto", count);
    return count;
}

bool LettersServer::is_letter(char c) {
    // Considera letras ASCII básicas (a-z, A-Z)
    return std::isalpha(static_cast<unsigned char>(c));
}