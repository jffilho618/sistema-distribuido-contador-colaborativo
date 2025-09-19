#include "master_server.h"
#include "logger.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <algorithm>
#include <future>
#include <functional>

using json = nlohmann::json;

MasterServer::MasterServer(int server_port) : port(server_port), running(false) {
    Logger::info_f("Servidor mestre criado na porta %d", port);
}

MasterServer::~MasterServer() {
    stop();
}

void MasterServer::add_slave(const std::string& name, const std::string& host, int port,
                            const std::string& endpoint, const std::string& type) {
    auto slave = std::make_unique<SlaveInfo>(name, host, port, endpoint, type);
    slaves.push_back(std::move(slave));
    Logger::info_f("Escravo adicionado: %s (%s:%d) - Tipo: %s",
                  name.c_str(), host.c_str(), port, type.c_str());
}

bool MasterServer::start() {
    if (running.load()) {
        Logger::warning("Servidor já está rodando");
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
            response["service"] = "master";
            response["slaves_count"] = slaves.size();

            // Status dos escravos
            json slaves_status = json::array();
            for (const auto& slave : slaves) {
                json slave_info;
                slave_info["name"] = slave->name;
                slave_info["type"] = slave->type;
                slave_info["healthy"] = slave->is_healthy;
                slaves_status.push_back(slave_info);
            }
            response["slaves"] = slaves_status;

            res.set_content(response.dump(), "application/json");
            Logger::debug("Health check requisitado");
        });

        // Processamento principal
        server.Post("/process", [this](const httplib::Request& req, httplib::Response& res) {
            Logger::info_f("Cliente conectado ao servidor mestre de %s", req.remote_addr.c_str());
            Logger::info("Requisição de processamento recebida");

            try {
                json request_json = json::parse(req.body);
                std::string text = request_json["text"];

                Logger::info_f("Processando texto de %zu caracteres", text.length());

                auto start_time = std::chrono::high_resolution_clock::now();

                std::string result = process_text_request(text);

                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                // Adicionar tempo de processamento à resposta
                json result_json = json::parse(result);
                result_json["processing_time_ms"] = duration.count();

                res.set_content(result_json.dump(), "application/json");
                Logger::info_f("Processamento concluído em %ld ms", duration.count());

            } catch (const std::exception& e) {
                Logger::error_f("Erro no processamento: %s", e.what());

                json error_response;
                error_response["success"] = false;
                error_response["error_message"] = e.what();

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

        Logger::info_f("Iniciando servidor mestre na porta %d", port);
        running = true;

        // Health check inicial dos escravos
        update_slaves_health();

        Logger::info("Rotas configuradas no servidor mestre");

        // Iniciar servidor (bloqueia thread)
        bool started = server.listen("0.0.0.0", port);

        if (!started) {
            Logger::error("Falha ao iniciar servidor HTTP");
            running = false;
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error_f("Erro ao iniciar servidor: %s", e.what());
        running = false;
        return false;
    }
}

void MasterServer::stop() {
    if (running.load()) {
        Logger::info("Parando servidor mestre");
        running = false;
    }
}

bool MasterServer::is_running() const {
    return running.load();
}

std::string MasterServer::process_text_request(const std::string& text) {
    json result;
    result["success"] = false;
    result["letters_count"] = 0;
    result["numbers_count"] = 0;
    result["total_characters"] = text.length();
    result["error_message"] = "";

    try {
        // Encontrar escravos saudáveis por tipo
        SlaveInfo* letters_slave = nullptr;
        SlaveInfo* numbers_slave = nullptr;

        for (const auto& slave : slaves) {
            if (slave->type == "letters" && slave->is_healthy) {
                letters_slave = slave.get();
            } else if (slave->type == "numbers" && slave->is_healthy) {
                numbers_slave = slave.get();
            }
        }

        if (!letters_slave) {
            throw std::runtime_error("Nenhum escravo de letras disponível");
        }
        if (!numbers_slave) {
            throw std::runtime_error("Nenhum escravo de números disponível");
        }

        // Delegar processamento para escravos EM PARALELO usando threads
        Logger::info("Iniciando processamento paralelo com threads");

        // Criar futures para execução paralela
        std::future<std::string> letters_future = std::async(std::launch::async,
            [this, letters_slave, &text]() {
                Logger::debug("Thread de letras iniciada");
                return delegate_to_slave(*letters_slave, text);
            });

        std::future<std::string> numbers_future = std::async(std::launch::async,
            [this, numbers_slave, &text]() {
                Logger::debug("Thread de números iniciada");
                return delegate_to_slave(*numbers_slave, text);
            });

        // Aguardar resultados das duas threads
        Logger::debug("Aguardando resultados das threads paralelas");
        std::string letters_result = letters_future.get();
        std::string numbers_result = numbers_future.get();
        Logger::info("Processamento paralelo concluído");

        // Combinar resultados
        json letters_json = json::parse(letters_result);
        json numbers_json = json::parse(numbers_result);

        if (letters_json["success"] && numbers_json["success"]) {
            result["success"] = true;
            result["letters_count"] = letters_json["count"];
            result["numbers_count"] = numbers_json["count"];

            Logger::info_f("Processamento distribuído concluído: %d letras, %d números",
                          (int)result["letters_count"], (int)result["numbers_count"]);
        } else {
            std::string error = "Erro nos escravos: ";
            if (!letters_json["success"]) {
                error += "letras(" + letters_json.value("error", "desconhecido") + ") ";
            }
            if (!numbers_json["success"]) {
                error += "números(" + numbers_json.value("error", "desconhecido") + ")";
            }
            result["error_message"] = error;
        }

    } catch (const std::exception& e) {
        result["error_message"] = e.what();
        Logger::error_f("Erro no processamento distribuído: %s", e.what());
    }

    return result.dump();
}

std::string MasterServer::delegate_to_slave(const SlaveInfo& slave, const std::string& data) {
    Logger::debug_f("Delegando para escravo %s (%s:%d)",
                   slave.name.c_str(), slave.host.c_str(), slave.port);

    try {
        httplib::Client client(slave.host, slave.port);
        client.set_connection_timeout(5, 0);
        client.set_read_timeout(15, 0);

        json request_data;
        request_data["text"] = data;

        httplib::Headers headers = {
            {"Content-Type", "application/json"}
        };

        auto response = client.Post(slave.endpoint.c_str(), headers,
                                   request_data.dump(), "application/json");

        if (!response) {
            throw std::runtime_error("Falha na conexão com escravo " + slave.name);
        }

        if (response->status != 200) {
            throw std::runtime_error("Escravo " + slave.name + " retornou status " +
                                   std::to_string(response->status));
        }

        Logger::debug_f("Resposta do escravo %s recebida", slave.name.c_str());
        return response->body;

    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["error"] = e.what();
        error_response["count"] = 0;

        Logger::error_f("Erro ao comunicar com escravo %s: %s",
                       slave.name.c_str(), e.what());

        return error_response.dump();
    }
}

bool MasterServer::check_slave_health(const SlaveInfo& slave) {
    try {
        httplib::Client client(slave.host, slave.port);
        client.set_connection_timeout(3, 0);
        client.set_read_timeout(5, 0);

        auto response = client.Get("/health");

        return response && response->status == 200;

    } catch (const std::exception&) {
        return false;
    }
}

void MasterServer::update_slaves_health() {
    Logger::debug("Atualizando status de saúde dos escravos");

    for (auto& slave : slaves) {
        bool old_status = slave->is_healthy;
        slave->is_healthy = check_slave_health(*slave);

        if (old_status != slave->is_healthy) {
            Logger::info_f("Escravo %s mudou status: %s -> %s",
                          slave->name.c_str(),
                          old_status ? "saudável" : "indisponível",
                          slave->is_healthy ? "saudável" : "indisponível");
        }
    }
}