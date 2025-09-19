#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Configurar aplicação
    app.setApplicationName("Cliente Sistema Distribuído");
    app.setApplicationVersion("2.0 Qt");
    app.setOrganizationName("Sistema Distribuído");

    // Configurar estilo
    app.setStyle(QStyleFactory::create("Fusion"));

    // Paleta de cores moderna
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    // Aplicar tema claro por padrão (pode ser mudado para escuro se preferir)
    // app.setPalette(darkPalette);

    try {
        // Criar e mostrar janela principal
        MainWindow window;
        window.show();

        return app.exec();

    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Erro Fatal",
                            QString("Erro fatal: %1").arg(e.what()));
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        QMessageBox::critical(nullptr, "Erro Fatal", "Erro desconhecido ocorreu");
        std::cerr << "Erro desconhecido ocorreu" << std::endl;
        return EXIT_FAILURE;
    }
}