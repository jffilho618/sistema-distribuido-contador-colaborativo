# 🚀 Sistema Distribuído - Contador de Letras e Números

[![Python](https://img.shields.io/badge/Python-3.7+-blue.svg)](https://python.org)
[![PyQt5](https://img.shields.io/badge/PyQt5-5.15+-green.svg)](https://pypi.org/project/PyQt5/)
[![Docker](https://img.shields.io/badge/Docker-20.10+-blue.svg)](https://docker.com)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Sistema distribuído moderno para análise de texto que conta letras e números utilizando arquitetura **mestre-escravo** com múltiplos serviços especializados.

## 📑 Índice

- [Visão Geral](#-visão-geral)
- [Arquitetura](#-arquitetura)
- [Funcionalidades](#-funcionalidades)
- [Pré-requisitos](#-pré-requisitos)
- [Instalação e Execução](#-instalação-e-execução)
- [Uso em Rede](#-uso-em-rede)
- [Exemplos de Uso](#-exemplos-de-uso)
- [API Reference](#-api-reference)
- [Solução de Problemas](#-solução-de-problemas)
- [Contribuição](#-contribuição)

## 🎯 Visão Geral

Este projeto implementa um sistema distribuído que processa texto de forma eficiente, dividindo a tarefa de contagem entre serviços especializados:

- **🎛️ Mestre**: Coordena requisições e agrega resultados
- **🔤 Escravo de Letras**: Conta exclusivamente letras do alfabeto
- **🔢 Escravo de Números**: Conta exclusivamente dígitos numéricos
- **🖥️ Cliente**: Interface gráfica para interação com o usuário

### 🌐 Arquitetura de Rede

```
Máquina A (Servidor)          Máquina B (Cliente)
┌─────────────────────┐       ┌─────────────────────┐
│  🐳 Docker Compose  │  ⟷  │  🖥️ Cliente Qt      │
│   🎛️ Master (8080)  │      │                      │
│   📝 Slave-Letters  │      │  Enviará arquivos    │
│   🔢 Slave-Numbers  │      │  via HTTP REST       │
└─────────────────────┘       └─────────────────────┘
      IP: 192.168.1.100             IP: 192.168.1.101
```

## 🏗️ Arquitetura

### Componentes do Sistema

```mermaid
graph TB
    Client[Cliente Qt] --> Master[Servidor Mestre :8080]
    Master --> |Thread Paralela| SlaveL[Escravo Letras :8081]
    Master --> |Thread Paralela| SlaveN[Escravo Números :8082]

    Master --> |Agrega Resultados| Client
    SlaveL --> |Contagem Letras| Master
    SlaveN --> |Contagem Números| Master
```

> **⚡ Paralelismo**: O mestre utiliza `std::async` para criar threads paralelas, enviando requisições simultaneamente aos escravos, melhorando significativamente a performance.

### Tecnologias Utilizadas

| Componente          | Tecnologia        | Linguagem | Porta |
| ------------------- | ----------------- | --------- | ----- |
| **Servidor Mestre** | C++ + httplib     | C++       | 8080  |
| **Escravo Letras**  | C++ + httplib     | C++       | 8081  |
| **Escravo Números** | C++ + httplib     | C++       | 8082  |
| **Cliente Qt**      | Qt6/Qt5 + httplib | C++       | -     |
| **Orquestração**    | Docker Compose    | YAML      | -     |

## ⚡ Funcionalidades

### 🖥️ Interface do Servidor

![Servidor](docs/images/servidor.png)
_Tela do servidor mostrando logs e status dos serviços_

### 🖱️ Interface do Cliente

![Cliente](docs/images/cliente.jpg)
_Interface gráfica do cliente com processamento de texto_

### Recursos Principais

- ✅ **Processamento Distribuído**: Divisão inteligente de tarefas
- ✅ **Processamento Paralelo**: Threads simultâneas para escravos
- ✅ **Interface Gráfica**: Cliente Qt moderno e intuitivo
- ✅ **Processamento Assíncrono**: Não bloqueia a interface durante operações
- ✅ **Suporte a Arquivos**: Processa arquivos de texto de qualquer tamanho
- ✅ **Entrada Manual**: Digite texto diretamente na interface
- ✅ **Estatísticas Detalhadas**: Contagens e percentuais precisos
- ✅ **Tempo de Resposta**: Métricas de performance em tempo real
- ✅ **Health Check**: Monitoramento automático dos serviços
- ✅ **Logs Estruturados**: Sistema de logging completo
- ✅ **Tolerância a Falhas**: Recuperação automática de erros

## ⚙️ Pré-requisitos

### Para o Servidor (Docker)

- **Docker Engine** 20.10+
- **Docker Compose** 2.0+
- **Porta 8080, 8081, 8082** disponíveis

### Para o Cliente

- **Qt6** ou **Qt5** (Core, Widgets, Network)
- **CMake** 3.16+
- **Compilador C++17** (GCC, Clang, MSVC)

### Verificação dos Pré-requisitos

```bash
# Verificar Docker
docker --version
docker-compose --version

# Verificar Qt e CMake
qmake --version
cmake --version
gcc --version

# Verificar portas disponíveis (Windows)
netstat -an | findstr "8080\|8081\|8082"
```

## 🚀 Instalação e Execução

### 1️⃣ Clonar o Repositório

```bash
git clone https://github.com/seu-usuario/mestre-escravo-sistemas-distribuidos.git
cd mestre-escravo-sistemas-distribuidos
```

### 2️⃣ Executar o Servidor (Docker)

```bash
# Construir e iniciar todos os serviços
docker-compose up --build -d

# Verificar se todos os serviços estão rodando
docker-compose ps

# Visualizar logs (opcional)
docker-compose logs -f
```

### 3️⃣ Executar o Cliente

#### 🖥️ Cliente Qt (C++)

```bash
# Navegar para o diretório do cliente
cd client

# Opção 1: Usar qmake (Qt Project)
qmake client.pro
make
./client

# Opção 2: Usar CMake
mkdir build && cd build
cmake ..
make
./client

# Windows (usando qmake)
qmake client.pro
mingw32-make
.\release\client.exe

# Windows (usando build.bat)
build.bat
```

### 4️⃣ Verificar Funcionamento

```bash
# Testar servidor diretamente
curl http://localhost:8080/health

# Resposta esperada:
# {"status": "healthy", "timestamp": "2024-01-01T12:00:00Z"}
```

## 🌐 Uso em Rede

### Configuração para Múltiplas Máquinas

#### 🖥️ Máquina Servidor (com Docker)

1. **Descobrir IP da máquina:**

   ```bash
   # Windows
   ipconfig

   # Linux/Mac
   ip addr show
   ```

2. **Anotar o IP** (exemplo: `192.168.1.100`)

3. **Configurar firewall** (se necessário):

   ```bash
   # Windows (executar como Admin)
   netsh advfirewall firewall add rule name="Docker Ports" dir=in action=allow protocol=TCP localport=8080-8082

   # Linux
   sudo ufw allow 8080:8082/tcp
   ```

#### 💻 Máquina Cliente

1. **Copiar pasta client** para a máquina cliente

2. **Compilar cliente Qt:**

   ```bash
   # CMake (recomendado)
   mkdir build && cd build
   cmake ..
   make

   # Ou qmake
   qmake client.pro
   make
   ```

3. **Executar cliente apontando para servidor:**

   ```bash
   # Executar cliente
   ./client

   # Na interface gráfica, configurar:
   # IP: 192.168.1.100, Porta: 8080
   ```

### 🔗 Teste de Conectividade

```bash
# Na máquina cliente, testar conectividade
ping 192.168.1.100

# Testar serviço
curl http://192.168.1.100:8080/health
```

## 📋 Exemplos de Uso

### Exemplo 1: Processamento de Texto Simples

**Entrada:**

```
Olá mundo! Tenho 25 anos e 3 gatos.
```

**Resultado:**

```
============================================================
                 RESULTADO DO PROCESSAMENTO
 Fonte: Texto (34 caracteres)
============================================================
✅ Processamento concluído com sucesso!

📊 ESTATÍSTICAS:
   Letras encontradas:     24
   Números encontrados:     2
   Total de caracteres:    34
   Tempo de processamento: 45.67 ms

📈 DISTRIBUIÇÃO:
   Letras:  92.31%
   Números:  7.69%
============================================================
```

### Exemplo 2: Processamento de Arquivo

**Arquivo: exemplo.txt**

```
Sistema Distribuído - Versão 2.0
Processando 1000 caracteres em 3 serviços.
Performance: 99.9% de uptime.
```

**Comando:**

```bash
# Selecionar arquivo via interface gráfica
# Ou usar botão "Processar Arquivo"
```

## 🔌 API Reference

### Endpoints Disponíveis

#### `GET /health`

Verificar status do servidor

**Resposta:**

```json
{
  "status": "healthy",
  "timestamp": "2024-01-01T12:00:00Z",
  "slaves": {
    "letters": "healthy",
    "numbers": "healthy"
  }
}
```

#### `POST /process`

Processar texto

**Request:**

```json
{
  "text": "Exemplo de texto com 123 números"
}
```

**Response:**

```json
{
  "success": true,
  "letters_count": 25,
  "numbers_count": 3,
  "total_characters": 35,
  "processing_time_ms": 42.5
}
```

## 🔧 Solução de Problemas

### Problemas Comuns

#### ❌ "Conexão recusada"

```bash
# Verificar se Docker está rodando
docker ps

# Verificar portas
netstat -an | grep 8080

# Verificar logs
docker-compose logs master
```

#### ❌ "Timeout na conexão"

- ✅ Verificar firewall em ambas as máquinas
- ✅ Confirmar se estão na mesma rede
- ✅ Testar ping entre máquinas

#### ❌ "Cliente não encontra Qt"

```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev cmake

# CentOS/RHEL
sudo yum install qt6-qtbase-devel cmake

# Windows (usando chocolatey)
choco install qt-creator-opensource

# macOS (usando homebrew)
brew install qt6 cmake
```

## 🤝 Contribuição

### Estrutura do Projeto

```
mestre-escravo-sistemas-distribuidos/
├── 📁 master/              # Servidor mestre (C++)
│   ├── src/
│   ├── CMakeLists.txt
│   └── Dockerfile
├── 📁 slave_letters/       # Escravo de letras (C++)
│   ├── src/
│   ├── CMakeLists.txt
│   └── Dockerfile
├── 📁 slave_numbers/       # Escravo de números (C++)
│   ├── src/
│   ├── CMakeLists.txt
│   └── Dockerfile
├── 📁 client/              # Cliente Qt (C++)
│   ├── src/                # Código fonte Qt
│   │   ├── main.cpp
│   │   ├── mainwindow.cpp
│   │   ├── mainwindow.h
│   │   ├── httpclient.cpp
│   │   ├── httpclient.h
│   │   ├── fileprocessor.cpp
│   │   └── fileprocessor.h
│   ├── client.pro          # Qt Project File
│   ├── CMakeLists.txt      # Build CMake
│   ├── build.sh           # Script build Linux/macOS
│   ├── build.bat          # Script build Windows
│   ├── input_files/       # Arquivos de exemplo
│   └── release/           # Executável compilado
├── 📁 docs/                # Documentação e imagens
│   └── images/
├── docker-compose.yml      # Orquestração dos serviços
└── README.md
```

## 📄 Licença

Este projeto está licenciado sob a **MIT License**.

---

**Desenvolvido com ❤️ para sistemas distribuídos modernos**
