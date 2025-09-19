#include "numbers_server.h"
#include "logger.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <cctype>
#include <chrono>

using json = nlohmann::json;

NumbersServer::NumbersServer(int server_port) : port(server_port), running(false) {
    Logger::info_f("Servidor de números criado na porta %d", port);
}

NumbersServer::~NumbersServer() {
    stop();
}

bool NumbersServer::start() {
    if (running.load()) {
        Logger::warning("Servidor de números já está rodando");
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
            response["service"] = "slave-numbers";
            response["port"] = port;

            res.set_content(response.dump(), "application/json");
            Logger::debug("Health check requisitado no servidor de números");
        });

        // Endpoint principal para contar números
        server.Post("/numeros", [this](const httplib::Request& req, httplib::Response& res) {
            Logger::info_f("Servidor mestre conectado ao escravo de números de %s", req.remote_addr.c_str());
            Logger::info("Requisição de contagem de números recebida");

            try {
                json request_json = json::parse(req.body);
                std::string text = request_json["text"];

                Logger::debug_f("Processando texto de %zu caracteres para números", text.length());

                auto start_time = std::chrono::high_resolution_clock::now();

                std::string result = process_numbers_request(text);

                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                // Adicionar tempo de processamento
                json result_json = json::parse(result);
                result_json["processing_time_ms"] = duration.count();

                res.set_content(result_json.dump(), "application/json");
                Logger::debug_f("Contagem de números concluída em %ld ms", duration.count());

            } catch (const std::exception& e) {
                Logger::error_f("Erro na contagem de números: %s", e.what());

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

        Logger::info_f("Iniciando servidor de números na porta %d", port);
        running = true;

        Logger::info("Rotas configuradas no servidor de números");

        // Iniciar servidor (bloqueia thread)
        bool started = server.listen("0.0.0.0", port);

        if (!started) {
            Logger::error("Falha ao iniciar servidor HTTP de números");
            running = false;
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error_f("Erro ao iniciar servidor de números: %s", e.what());
        running = false;
        return false;
    }
}

void NumbersServer::stop() {
    if (running.load()) {
        Logger::info("Parando servidor de números");
        running = false;
    }
}

bool NumbersServer::is_running() const {
    return running.load();
}

std::string NumbersServer::process_numbers_request(const std::string& text) {
    json result;

    try {
        int number_count = count_numbers(text);

        result["success"] = true;
        result["count"] = number_count;
        result["service"] = "numbers";
        result["processed_characters"] = text.length();

        Logger::info_f("Contagem de números concluída: %d números em %zu caracteres",
                      number_count, text.length());

    } catch (const std::exception& e) {
        result["success"] = false;
        result["error"] = e.what();
        result["count"] = 0;

        Logger::error_f("Erro no processamento de números: %s", e.what());
    }

    return result.dump();
}

int NumbersServer::count_numbers(const std::string& text) {
    int count = 0;

    for (char c : text) {
        if (is_number(c)) {
            count++;
        }
    }

    Logger::debug_f("Contados %d números no texto", count);
    return count;
}

bool NumbersServer::is_number(char c) {
    // Considera dígitos ASCII (0-9)
    return std::isdigit(static_cast<unsigned char>(c));
}