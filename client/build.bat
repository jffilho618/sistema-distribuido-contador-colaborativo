@echo off
REM Script de build para o Cliente C++ Qt (Windows)
REM Sistema Distribuído - Contador de Letras/Números

echo 🚀 Cliente C++ Qt - Sistema Distribuído (Windows)
echo ==============================================

REM Verificar se estamos no diretório correto
if not exist "client.pro" (
    if not exist "CMakeLists.txt" (
        echo ❌ Nem client.pro nem CMakeLists.txt encontrados. Execute este script do diretório client/
        pause
        exit /b 1
    )
)

echo ℹ️  Verificando dependências...

REM Verificar qmake primeiro
qmake -version >nul 2>&1
if not errorlevel 1 (
    echo ✅ Qt encontrado via qmake
    set BUILD_METHOD=qmake
    goto build
)

REM Verificar CMake se qmake não disponível
cmake --version >nul 2>&1
if not errorlevel 1 (
    echo ✅ CMake encontrado, Qt não disponível
    set BUILD_METHOD=cmake
    goto build
)

echo ❌ Nem Qt nem CMake estão instalados
echo Instale o Qt: https://www.qt.io/download
echo Ou CMake: https://cmake.org/download/
pause
exit /b 1

:build

if "%BUILD_METHOD%"=="qmake" (
    echo ℹ️  Compilando com Qt...

    REM Limpar arquivos anteriores
    if exist "Makefile" (
        echo ℹ️  Limpando build anterior...
        del /q Makefile* 2>nul
    )

    REM Gerar Makefile
    echo ℹ️  Gerando Makefile...
    qmake client.pro

    if errorlevel 1 (
        echo ❌ Falha ao gerar Makefile
        echo Verifique se o Qt está corretamente instalado
        pause
        exit /b 1
    )

    REM Compilar
    echo ℹ️  Compilando aplicação...
    nmake release 2>nul || make release 2>nul || mingw32-make release 2>nul

    REM Verificar resultado
    if exist "release\client.exe" (
        echo ✅ Compilação concluída com sucesso!
        echo.
        echo 🎉 Cliente Qt compilado!
        echo 📍 Executável: %cd%\release\client.exe
        echo.
        echo Para executar:
        echo   release\client.exe
    ) else if exist "client.exe" (
        echo ✅ Compilação concluída com sucesso!
        echo.
        echo 🎉 Cliente Qt compilado!
        echo 📍 Executável: %cd%\client.exe
        echo.
        echo Para executar:
        echo   client.exe
    ) else (
        echo ❌ Falha na compilação
        echo Verifique se todas as dependências Qt estão instaladas
        pause
        exit /b 1
    )

) else (
    echo ℹ️  Compilando com CMake...

    REM Criar diretório de build
    if not exist "build" mkdir build
    cd build

    REM Limpar build anterior
    if exist "*.sln" (
        echo ℹ️  Limpando build anterior...
        del /q * 2>nul
    )

    REM Configurar build
    echo ℹ️  Configurando build com CMake...
    cmake .. -DCMAKE_BUILD_TYPE=Release

    if errorlevel 1 (
        echo ❌ Falha na configuração CMake
        echo Verifique se o Qt está corretamente instalado
        pause
        exit /b 1
    )

    REM Compilar
    echo ℹ️  Compilando aplicação...
    cmake --build . --config Release

    REM Verificar resultado
    if exist "Release\client.exe" (
        echo ✅ Compilação concluída com sucesso!
        echo.
        echo 🎉 Cliente Qt compilado!
        echo 📍 Executável: %cd%\Release\client.exe
        echo.
        echo Para executar:
        echo   Release\client.exe
    ) else if exist "client.exe" (
        echo ✅ Compilação concluída com sucesso!
        echo.
        echo 🎉 Cliente Qt compilado!
        echo 📍 Executável: %cd%\client.exe
        echo.
        echo Para executar:
        echo   client.exe
    ) else (
        echo ❌ Falha na compilação
        echo Verifique os erros acima
        pause
        exit /b 1
    )
)

pause