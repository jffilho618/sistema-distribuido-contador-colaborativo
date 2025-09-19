# Dockerfile para o Escravo de Letras
FROM ubuntu:22.04

# Evitar prompts interativos durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependências do sistema
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Criar diretório de trabalho
WORKDIR /app

# Copiar arquivos de build
COPY CMakeLists.txt .
COPY src/ ./src/

# Compilar a aplicação
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Configurar usuário não-root para segurança
RUN useradd -m -u 1000 appuser && \
    chown -R appuser:appuser /app
USER appuser

# Expor porta do serviço
EXPOSE 8081

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8081/health || exit 1

# Comando padrão
CMD ["./build/slave-letters"]

# Labels para metadados
LABEL maintainer="Sistema Distribuído C++"
LABEL version="1.0"
LABEL description="Escravo para processamento de letras"