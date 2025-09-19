cmake_minimum_required(VERSION 3.16)
project(SlaveLetters)

# Configura��es do C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configura��es de debug/release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Encontrar depend�ncias
find_package(Threads REQUIRED)

# Verificar se httplib est� dispon�vel
include(CheckIncludeFileCXX)
check_include_file_cxx("httplib.h" HAVE_HTTPLIB_H)

if(NOT HAVE_HTTPLIB_H)
    # Baixar httplib se n�o estiver dispon�vel
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

# Verificar se nlohmann_json est� dispon�vel
check_include_file_cxx("nlohmann/json.hpp" HAVE_NLOHMANN_JSON_H)

if(NOT HAVE_NLOHMANN_JSON_H)
    # Baixar nlohmann_json se n�o estiver dispon�vel
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
set(SLAVE_SOURCES
    src/main.cpp
    src/letters_server.cpp
    src/logger.cpp
)

# Execut�vel
add_executable(slave-letters ${SLAVE_SOURCES})

# Headers
target_include_directories(slave-letters PRIVATE src)

# Linkar bibliotecas
target_link_libraries(slave-letters PRIVATE
    Threads::Threads
    ${HTTPLIB_TARGET}
    ${JSON_TARGET}
)

# Defini��es do compilador
target_compile_definitions(slave-letters PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT=0
    CPPHTTPLIB_ZLIB_SUPPORT=0
)

# Instalar
install(TARGETS slave-letters DESTINATION bin)