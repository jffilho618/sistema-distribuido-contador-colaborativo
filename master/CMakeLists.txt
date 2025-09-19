cmake_minimum_required(VERSION 3.16)
project(MasterServer)

# Configurações do C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configurações de debug/release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Encontrar dependências
find_package(Threads REQUIRED)

# Verificar se httplib está disponível
include(CheckIncludeFileCXX)
check_include_file_cxx("httplib.h" HAVE_HTTPLIB_H)

if(NOT HAVE_HTTPLIB_H)
    # Baixar httplib se não estiver disponível
    include(FetchContent)
    FetchContent_Declare(
        httplib
        URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.14.1.tar.gz
    )
    FetchContent_MakeAvailable(httplib)
    set(HTTPLIB_TARGET httplib::httplib)
else()
    set(HTTPLIB_TARGET "")
endif()

# Verificar se nlohmann_json está disponível
check_include_file_cxx("nlohmann/json.hpp" HAVE_NLOHMANN_JSON_H)

if(NOT HAVE_NLOHMANN_JSON_H)
    # Baixar nlohmann_json se não estiver disponível
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz
    )
    FetchContent_MakeAvailable(nlohmann_json)
    set(JSON_TARGET nlohmann_json::nlohmann_json)
else()
    set(JSON_TARGET "")
endif()

# Arquivos fonte
set(MASTER_SOURCES
    src/main.cpp
    src/master_server.cpp
    src/logger.cpp
)

# Executável
add_executable(master ${MASTER_SOURCES})

# Headers
target_include_directories(master PRIVATE src)

# Linkar bibliotecas
target_link_libraries(master PRIVATE 
    Threads::Threads
    ${HTTPLIB_TARGET}
    ${JSON_TARGET}
)

# Definições do compilador
target_compile_definitions(master PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT=0
    CPPHTTPLIB_ZLIB_SUPPORT=0
)

# Instalar
install(TARGETS master DESTINATION bin)