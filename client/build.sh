#!/bin/bash

# Script de build para o Cliente C++ Qt
# Sistema Distribuído - Contador de Letras/Números

set -e  # Parar em caso de erro

echo "🚀 Cliente C++ Qt - Sistema Distribuído"
echo "======================================"

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Função para imprimir mensagens coloridas
print_info() {
    echo -e "${YELLOW}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Verificar se estamos no diretório correto
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt não encontrado. Execute este script do diretório client/"
    exit 1
fi

print_info "Verificando dependências..."

# Verificar CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake não está instalado"
    echo "Ubuntu/Debian: sudo apt install cmake"
    echo "CentOS/RHEL: sudo yum install cmake"
    echo "macOS: brew install cmake"
    exit 1
fi

# Verificar qmake
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -query QT_VERSION)
    print_success "Qt $QT_VERSION encontrado (qmake)"
    BUILD_METHOD="qmake"
elif command -v qmake6 &> /dev/null; then
    QT_VERSION=$(qmake6 -query QT_VERSION)
    print_success "Qt $QT_VERSION encontrado (qmake6)"
    BUILD_METHOD="qmake6"
elif command -v cmake &> /dev/null; then
    print_info "qmake não encontrado, tentando CMake..."
    BUILD_METHOD="cmake"
else
    print_error "Nem Qt nem CMake estão instalados"
    echo "Ubuntu/Debian: sudo apt install qtbase5-dev qttools5-dev-tools"
    echo "                 ou sudo apt install qt6-base-dev qt6-tools-dev"
    echo "CentOS/RHEL: sudo yum install qt5-qtbase-devel qt5-qttools-devel"
    echo "macOS: brew install qt"
    exit 1
fi

print_info "Dependências verificadas com sucesso!"
print_info "Método de build: $BUILD_METHOD"

# Compilar baseado no método disponível
if [ "$BUILD_METHOD" = "qmake" ] || [ "$BUILD_METHOD" = "qmake6" ]; then
    print_info "Compilando com Qt ($BUILD_METHOD)..."

    # Limpar build anterior
    if [ -f "Makefile" ]; then
        print_info "Limpando build anterior..."
        make clean 2>/dev/null || true
        rm -f Makefile
    fi

    # Gerar Makefile
    print_info "Gerando Makefile..."
    if [ "$BUILD_METHOD" = "qmake6" ]; then
        qmake6 ../client.pro
    else
        qmake ../client.pro
    fi

    # Compilar
    print_info "Compilando aplicação..."
    if command -v nproc &> /dev/null; then
        CORES=$(nproc)
    elif command -v sysctl &> /dev/null; then
        CORES=$(sysctl -n hw.ncpu)
    else
        CORES=2
    fi

    make -j$CORES

    # Verificar resultado
    if [ -f "client" ] || [ -f "release/client" ] || [ -f "debug/client" ]; then
        print_success "Compilação concluída com sucesso!"
        echo
        echo "🎉 Cliente Qt compilado!"

        # Encontrar executável
        if [ -f "client" ]; then
            echo "📍 Executável: $(pwd)/client"
            EXEC_PATH="./client"
        elif [ -f "release/client" ]; then
            echo "📍 Executável: $(pwd)/release/client"
            EXEC_PATH="./release/client"
        else
            echo "📍 Executável: $(pwd)/debug/client"
            EXEC_PATH="./debug/client"
        fi

        echo
        echo "Para executar:"
        echo "  $EXEC_PATH"
    else
        print_error "Falha na compilação"
        exit 1
    fi

else
    # Fallback para CMake
    print_info "Compilando com CMake..."

    # Criar diretório se não existir
    mkdir -p build
    cd build

    # Limpar build anterior
    if [ -f "Makefile" ]; then
        print_info "Limpando build anterior..."
        make clean 2>/dev/null || true
    fi

    # Configurar e compilar
    print_info "Configurando build com CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release

    print_info "Compilando aplicação..."
    if command -v nproc &> /dev/null; then
        CORES=$(nproc)
    elif command -v sysctl &> /dev/null; then
        CORES=$(sysctl -n hw.ncpu)
    else
        CORES=2
    fi

    make -j$CORES

    # Verificar resultado
    if [ -f "client" ]; then
        print_success "Compilação concluída com sucesso!"
        echo
        echo "🎉 Cliente Qt compilado!"
        echo "📍 Executável: $(pwd)/client"
        echo
        echo "Para executar:"
        echo "  ./client"
    else
        print_error "Falha na compilação"
        exit 1
    fi
fi