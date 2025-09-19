#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>

// Estrutura para informações do escravo
struct SlaveInfo {
    std::string name;
    std::string host;
    int port;
    std::string endpoint;
    std::string type;
    bool is_healthy = false;

    SlaveInfo(const std::string& n, const std::string& h, int p,
              const std::string& e, const std::string& t)
        : name(n), host(h), port(p), endpoint(e), type(t) {}
};

// Servidor mestre para coordenação dos escravos
class MasterServer {
private:
    int port;
    std::atomic<bool> running;
    std::vector<std::unique_ptr<SlaveInfo>> slaves;

public:
    explicit MasterServer(int server_port);
    ~MasterServer();

    // Gerenciamento de escravos
    void add_slave(const std::string& name, const std::string& host, int port,
                   const std::string& endpoint, const std::string& type);

    // Controle do servidor
    bool start();
    void stop();
    bool is_running() const;

    // Health checks
    bool check_slave_health(const SlaveInfo& slave);
    void update_slaves_health();

private:
    // Métodos auxiliares
    std::string process_text_request(const std::string& text);
    std::string delegate_to_slave(const SlaveInfo& slave, const std::string& data);
};