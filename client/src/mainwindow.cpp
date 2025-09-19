#include "mainwindow.h"
#include <QGridLayout>
#include <QStandardPaths>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QColor>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , httpClient(new HttpClient(this))
    , isProcessing(false)
    , lastUsedDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
{
    setupUI();
    connectSignals();

    // Configurar cliente HTTP padrão
    httpClient->setServer("localhost", 8080);

    // Mostrar mensagem inicial
    updateResults("Cliente iniciado.\nServidor configurado: http://localhost:8080\n\nPronto para processar arquivos e textos.");
    updateStatus("Pronto. Servidor: http://localhost:8080");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Cliente Sistema Distribuído - Contador de Letras/Números (Qt)");
    setMinimumSize(900, 700);
    resize(1000, 800);

    // Widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Criar grupos da interface
    createServerConfigGroup();
    createProcessingGroup();
    createTextInputGroup();
    createResultsGroup();
    createStatusBar();

    // Adicionar aos layouts
    mainLayout->addWidget(serverConfigGroup);
    mainLayout->addWidget(processingGroup);

    // Splitter para texto e resultados
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(textInputGroup);
    splitter->addWidget(resultsGroup);
    splitter->setSizes({200, 400});
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    mainLayout->addWidget(splitter);

    // Aplicar estilo
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            padding: 6px 12px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: #f8f9fa;
        }
        QPushButton:hover {
            background-color: #e2e6ea;
            border-color: #adb5bd;
        }
        QPushButton:pressed {
            background-color: #dee2e6;
        }
        QPushButton:disabled {
            background-color: #e9ecef;
            color: #6c757d;
        }
        QTextEdit {
            border: 1px solid #ced4da;
            border-radius: 4px;
            padding: 8px;
            font-family: "Consolas", "Monaco", "Courier New", monospace;
            font-size: 11pt;
        }
        QLineEdit, QSpinBox {
            padding: 4px 8px;
            border: 1px solid #ced4da;
            border-radius: 4px;
        }
    )");
}

void MainWindow::createServerConfigGroup()
{
    serverConfigGroup = new QGroupBox("Configuração do Servidor", this);
    QHBoxLayout* layout = new QHBoxLayout(serverConfigGroup);

    // Host
    layout->addWidget(new QLabel("Servidor:"));
    hostLineEdit = new QLineEdit("localhost");
    hostLineEdit->setFixedWidth(150);
    layout->addWidget(hostLineEdit);

    // Porta
    layout->addWidget(new QLabel("Porta:"));
    portSpinBox = new QSpinBox();
    portSpinBox->setRange(1, 65535);
    portSpinBox->setValue(8080);
    portSpinBox->setFixedWidth(80);
    layout->addWidget(portSpinBox);

    // Botões
    configButton = new QPushButton("Configurar");
    statusButton = new QPushButton("Verificar Status");
    layout->addWidget(configButton);
    layout->addWidget(statusButton);

    layout->addStretch();
}

void MainWindow::createProcessingGroup()
{
    processingGroup = new QGroupBox("Processamento", this);
    QHBoxLayout* layout = new QHBoxLayout(processingGroup);

    fileButton = new QPushButton("📁 Processar Arquivo");
    textButton = new QPushButton("📝 Processar Texto da Área");

    layout->addWidget(fileButton);
    layout->addWidget(textButton);
    layout->addStretch();
}

void MainWindow::createTextInputGroup()
{
    textInputGroup = new QGroupBox("Entrada de Texto", this);
    QVBoxLayout* layout = new QVBoxLayout(textInputGroup);

    layout->addWidget(new QLabel("Digite o texto a ser processado:"));

    textInput = new QTextEdit();
    textInput->setPlaceholderText("Digite seu texto aqui...");
    textInput->setMaximumHeight(150);
    layout->addWidget(textInput);
}

void MainWindow::createResultsGroup()
{
    resultsGroup = new QGroupBox("Resultados", this);
    QVBoxLayout* layout = new QVBoxLayout(resultsGroup);

    resultsDisplay = new QTextEdit();
    resultsDisplay->setReadOnly(true);
    resultsDisplay->setFont(QFont("Consolas", 10));
    layout->addWidget(resultsDisplay);
}

void MainWindow::createStatusBar()
{
    QStatusBar* status = statusBar();

    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    progressBar->setFixedWidth(200);
    status->addPermanentWidget(progressBar);

    statusLabel = new QLabel("Pronto");
    status->addWidget(statusLabel);

    progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &MainWindow::updateProcessingProgress);
}

void MainWindow::connectSignals()
{
    // Botões da interface
    connect(configButton, &QPushButton::clicked, this, &MainWindow::configureServer);
    connect(statusButton, &QPushButton::clicked, this, &MainWindow::checkServerStatus);
    connect(fileButton, &QPushButton::clicked, this, &MainWindow::processFile);
    connect(textButton, &QPushButton::clicked, this, &MainWindow::processText);

    // Cliente HTTP
    connect(httpClient, &HttpClient::healthCheckCompleted,
            this, &MainWindow::onHealthCheckCompleted);
    connect(httpClient, &HttpClient::textProcessingCompleted,
            this, &MainWindow::onTextProcessingCompleted);
    connect(httpClient, &HttpClient::networkError,
            this, &MainWindow::onNetworkError);
}

void MainWindow::configureServer()
{
    QString host = hostLineEdit->text().trimmed();
    int port = portSpinBox->value();

    if (host.isEmpty()) {
        showError("O campo servidor não pode estar vazio!");
        return;
    }

    httpClient->setServer(host, port);
    QString url = httpClient->getServerUrl();

    updateStatus("Servidor configurado: " + url);
    updateResults("Servidor configurado para: " + url);
}

void MainWindow::checkServerStatus()
{
    if (isProcessing) return;

    updateStatus("Verificando status do servidor...");
    setButtonsEnabled(false);
    showProcessingProgress(true);

    httpClient->checkServerHealth();
}

void MainWindow::processText()
{
    if (isProcessing) return;

    QString text = textInput->toPlainText().trimmed();
    if (text.isEmpty()) {
        showError("Texto não pode estar vazio!");
        return;
    }

    QString source = QString("Texto (%1 caracteres)").arg(text.length());
    processTextContent(text, source);
}

void MainWindow::processFile()
{
    if (isProcessing) return;

    QString filepath = selectFile();
    if (filepath.isEmpty()) {
        return;
    }

    try {
        QString content = FileProcessor::readFile(filepath);
        if (content.isEmpty()) {
            showError("Arquivo está vazio!");
            return;
        }

        QString filename = FileProcessor::getFileName(filepath);
        QString source = QString("Arquivo: %1").arg(filename);
        processTextContent(content, source);

        // Atualizar último diretório usado
        lastUsedDirectory = QFileInfo(filepath).absolutePath();

    } catch (const FileProcessingException& e) {
        showError("Erro ao ler arquivo: " + e.getMessage());
    }
}

void MainWindow::processTextContent(const QString& text, const QString& source)
{
    updateStatus("Processando " + source.toLower() + "...");
    setButtonsEnabled(false);
    showProcessingProgress(true);
    isProcessing = true;

    httpClient->processText(text);
}

void MainWindow::onHealthCheckCompleted(const ServerStatus& status)
{
    setButtonsEnabled(true);
    showProcessingProgress(false);

    QString url = httpClient->getServerUrl();
    if (status.online) {
        updateStatus("Servidor operacional: " + url);
        updateResults("✅ Servidor está operacional e pronto para processar requisições!\nURL: " + url);
    } else {
        updateStatus("Servidor não disponível: " + url);
        updateResults("❌ Servidor não está disponível ou com problemas.\nErro: " + status.error_message +
                    "\nVerifique se o servidor está rodando em: " + url);
    }
}

void MainWindow::onTextProcessingCompleted(const ProcessingResult& result)
{
    setButtonsEnabled(true);
    showProcessingProgress(false);
    isProcessing = false;

    QString formattedResult = formatResult(result, "processamento");
    updateResults(formattedResult);

    if (result.success) {
        updateStatus("Processamento concluído com sucesso!");
    } else {
        updateStatus("Erro no processamento: " + result.error_message);
    }
}

void MainWindow::onNetworkError(const QString& error)
{
    setButtonsEnabled(true);
    showProcessingProgress(false);
    isProcessing = false;

    updateStatus("Erro: " + error);
    updateResults("❌ Erro no processamento:\n" + error);
}

void MainWindow::updateProcessingProgress()
{
    static int value = 0;
    value = (value + 1) % 100;
    progressBar->setValue(value);
}

QString MainWindow::formatResult(const ProcessingResult& result, const QString& source)
{
    QString output;
    output += QString("=").repeated(60) + "\n";
    output += "RESULTADO DO PROCESSAMENTO\n";
    output += QString("Fonte: %1\n").arg(source);
    output += QString("=").repeated(60) + "\n\n";

    if (result.success) {
        output += "✅ Processamento concluído com sucesso!\n\n";

        output += "📊 ESTATÍSTICAS:\n";
        output += QString("   Letras encontradas:     %1\n").arg(result.letters_count, 8);
        output += QString("   Números encontrados:    %1\n").arg(result.numbers_count, 8);
        output += QString("   Total de caracteres:    %1\n").arg(result.total_characters, 8);
        output += QString("   Tempo de processamento: %1 ms\n").arg(result.processing_time_ms, 8, 'f', 2);

        int total = result.letters_count + result.numbers_count;
        if (total > 0) {
            double letterPct = (static_cast<double>(result.letters_count) / total) * 100.0;
            double numberPct = (static_cast<double>(result.numbers_count) / total) * 100.0;

            output += "\n📈 DISTRIBUIÇÃO:\n";
            output += QString("   Letras:  %1%\n").arg(letterPct, 6, 'f', 2);
            output += QString("   Números: %1%\n").arg(numberPct, 6, 'f', 2);
        }
    } else {
        output += "❌ Falha no processamento!\n\n";
        output += QString("💥 ERRO: %1\n").arg(result.error_message);
    }

    output += "\n" + QString("=").repeated(60);
    return output;
}

QString MainWindow::selectFile()
{
    QString filter = "Arquivos de texto (*.txt);;Todos os arquivos (*)";
    return QFileDialog::getOpenFileName(this,
                                       "Selecionar arquivo de texto",
                                       lastUsedDirectory,
                                       filter);
}

void MainWindow::clearTextInput()
{
    textInput->clear();
}

void MainWindow::updateStatus(const QString& message)
{
    statusLabel->setText(message);
}

void MainWindow::updateResults(const QString& results)
{
    resultsDisplay->setPlainText(results);

    // Rolar para o topo
    QTextCursor cursor = resultsDisplay->textCursor();
    cursor.movePosition(QTextCursor::Start);
    resultsDisplay->setTextCursor(cursor);
}

void MainWindow::setButtonsEnabled(bool enabled)
{
    configButton->setEnabled(enabled);
    statusButton->setEnabled(enabled);
    fileButton->setEnabled(enabled);
    textButton->setEnabled(enabled);
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Erro", message);
    updateStatus("Erro: " + message);
}

void MainWindow::showInfo(const QString& message)
{
    QMessageBox::information(this, "Informação", message);
}

void MainWindow::showProcessingProgress(bool show)
{
    progressBar->setVisible(show);
    if (show) {
        progressBar->setRange(0, 0); // Modo indeterminado
        progressTimer->start(50);
    } else {
        progressTimer->stop();
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
    }
}