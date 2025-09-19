QT += core widgets network

TARGET = client
TEMPLATE = app

# Configurações do C++
CONFIG += c++17
CONFIG += console

# Configurações de build
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
} else {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
}

# Definições
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# Incluir nlohmann/json se disponível
exists(/usr/include/nlohmann/json.hpp) {
    DEFINES += HAVE_NLOHMANN_JSON
}

# Arquivos fonte
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/httpclient.cpp \
    src/fileprocessor.cpp

# Arquivos cabeçalho
HEADERS += \
    src/mainwindow.h \
    src/httpclient.h \
    src/fileprocessor.h

# Arquivos de interface (se houver)
FORMS += \
    # src/mainwindow.ui

# Recursos (se houver)
RESOURCES += \
    # resources.qrc

# Ícone da aplicação (Windows) - comentado pois não temos o arquivo
# win32:RC_ICONS = app.ico

# Configurações específicas por plataforma
win32 {
    # Windows
    CONFIG += windows
    QMAKE_TARGET_COMPANY = "Sistema Distribuído"
    QMAKE_TARGET_PRODUCT = "Cliente Qt"
    QMAKE_TARGET_DESCRIPTION = "Cliente Sistema Distribuído - Contador de Letras/Números"
    QMAKE_TARGET_COPYRIGHT = "Copyright 2024"
}

unix:!macx {
    # Linux
    target.path = /usr/local/bin
    INSTALLS += target
}

macx {
    # macOS
    QMAKE_INFO_PLIST = Info.plist
    ICON = app.icns
}

# Bibliotecas externas
# Se usar vcpkg no Windows
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../vcpkg/installed/x64-windows/lib/
win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../vcpkg/installed/x64-windows/debug/lib/

# Diretórios de include
INCLUDEPATH += src
win32: INCLUDEPATH += $$PWD/../vcpkg/installed/x64-windows/include/

# Otimizações
QMAKE_CXXFLAGS_RELEASE += -O3

# Warnings
QMAKE_CXXFLAGS += -Wall -Wextra

# Output
message("Configurando build do Cliente Qt")
message("Qt version: $$[QT_VERSION]")
message("Target: $$TARGET")