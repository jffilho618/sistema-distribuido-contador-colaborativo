cmake_minimum_required(VERSION 3.16)
project(ClientApp)

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

# Encontrar Qt (tentar Qt6 primeiro, depois Qt5)
find_package(Qt6 QUIET COMPONENTS Core Widgets Network)
if(Qt6_FOUND)
    set(QT_VERSION 6)
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Network)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
    set(QT_VERSION 5)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Network)
    message(STATUS "Using Qt5")
endif()

# Habilitar MOC automático
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt já tem JSON nativo, mas vamos manter nlohmann para compatibilidade
include(CheckIncludeFileCXX)
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
set(CLIENT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/httpclient.cpp
    src/fileprocessor.cpp
)

# Headers Qt (para MOC)
set(CLIENT_HEADERS
    src/mainwindow.h
    src/httpclient.h
    src/fileprocessor.h
)

# Arquivos UI (se houver)
set(CLIENT_UI_FILES
    src/mainwindow.ui
)

# Executável
add_executable(client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

# Headers
target_include_directories(client PRIVATE src)

# Linkar bibliotecas
target_link_libraries(client PRIVATE
    Threads::Threads
    ${JSON_TARGET}
    ${QT_LIBRARIES}
)

# Instalar
install(TARGETS client DESTINATION bin)