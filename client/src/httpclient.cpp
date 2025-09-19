#include "httpclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>
#include <QEventLoop>
#include <QApplication>

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , serverHost("localhost")
    , serverPort(8080)
    , timeoutSeconds(30)
    , timeoutTimer(new QTimer(this))
    , currentReply(nullptr)
{
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &HttpClient::onRequestTimeout);
}

void HttpClient::setServer(const QString& host, int port)
{
    serverHost = host;
    serverPort = port;
}

void HttpClient::setTimeout(int seconds)
{
    timeoutSeconds = seconds;
}

QString HttpClient::getServerUrl() const
{
    return QString("http://%1:%2").arg(serverHost).arg(serverPort);
}

QNetworkRequest HttpClient::createRequest(const QString& endpoint)
{
    QUrl url(getServerUrl() + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "QtClient/1.0");
    return request;
}

void HttpClient::checkServerHealth()
{
    if (currentReply) {
        currentReply->abort();
        currentReply = nullptr;
    }

    QNetworkRequest request = createRequest("/health");
    currentReply = networkManager->get(request);

    connect(currentReply, &QNetworkReply::finished, this, &HttpClient::onHealthCheckFinished);

    requestTimer.start();
    timeoutTimer->start(timeoutSeconds * 1000);
}

void HttpClient::processText(const QString& text)
{
    if (currentReply) {
        currentReply->abort();
        currentReply = nullptr;
    }

    QNetworkRequest request = createRequest("/process");

    QJsonObject requestData;
    requestData["text"] = text;
    QJsonDocument doc(requestData);

    currentReply = networkManager->post(request, doc.toJson());

    connect(currentReply, &QNetworkReply::finished, this, &HttpClient::onProcessTextFinished);

    requestTimer.start();
    timeoutTimer->start(timeoutSeconds * 1000);
}

void HttpClient::onHealthCheckFinished()
{
    timeoutTimer->stop();

    if (!currentReply) {
        return;
    }

    ServerStatus status;
    status.online = false;
    status.error_message = "";

    if (currentReply->error() == QNetworkReply::NoError) {
        QByteArray data = currentReply->readAll();
        status = parseHealthResponse(data);
    } else {
        handleNetworkError(currentReply->error());
        status.error_message = currentReply->errorString();
    }

    currentReply->deleteLater();
    currentReply = nullptr;

    emit healthCheckCompleted(status);
}

void HttpClient::onProcessTextFinished()
{
    timeoutTimer->stop();

    if (!currentReply) {
        return;
    }

    ProcessingResult result;
    result.success = false;
    result.processing_time_ms = requestTimer.elapsed();

    if (currentReply->error() == QNetworkReply::NoError) {
        QByteArray data = currentReply->readAll();
        result = parseProcessResponse(data);
    } else {
        handleNetworkError(currentReply->error());
        result.error_message = currentReply->errorString();
    }

    currentReply->deleteLater();
    currentReply = nullptr;

    emit textProcessingCompleted(result);
}

void HttpClient::onRequestTimeout()
{
    if (currentReply) {
        currentReply->abort();
        currentReply = nullptr;
    }

    emit networkError("Timeout: Servidor não respondeu em tempo hábil");
}

ServerStatus HttpClient::parseHealthResponse(const QByteArray& data)
{
    ServerStatus status;
    status.online = false;

    try {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            status.error_message = "Erro ao analisar resposta JSON: " + error.errorString();
            return status;
        }

        QJsonObject obj = doc.object();
        status.online = true;
        status.status = obj.value("status").toString("unknown");
        status.timestamp = obj.value("timestamp").toString();

    } catch (const std::exception& e) {
        status.error_message = QString("Erro ao processar resposta: %1").arg(e.what());
    }

    return status;
}

ProcessingResult HttpClient::parseProcessResponse(const QByteArray& data)
{
    ProcessingResult result;
    result.success = false;
    result.letters_count = 0;
    result.numbers_count = 0;
    result.total_characters = 0;
    result.processing_time_ms = requestTimer.elapsed();

    try {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            result.error_message = "Erro ao analisar resposta JSON: " + error.errorString();
            return result;
        }

        QJsonObject obj = doc.object();
        result.success = obj.value("success").toBool(false);
        result.letters_count = obj.value("letters_count").toInt(0);
        result.numbers_count = obj.value("numbers_count").toInt(0);
        result.total_characters = obj.value("total_characters").toInt(0);

        // Usar tempo do servidor se disponível
        if (obj.contains("processing_time_ms")) {
            result.processing_time_ms = obj.value("processing_time_ms").toDouble();
        }

        if (!result.success) {
            result.error_message = obj.value("error_message").toString("Erro desconhecido no processamento");
        }

    } catch (const std::exception& e) {
        result.error_message = QString("Erro ao processar resposta: %1").arg(e.what());
    }

    return result;
}

void HttpClient::handleNetworkError(QNetworkReply::NetworkError error)
{
    QString errorMessage;

    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMessage = "Conexão recusada pelo servidor";
        break;
    case QNetworkReply::RemoteHostClosedError:
        errorMessage = "Servidor fechou a conexão";
        break;
    case QNetworkReply::HostNotFoundError:
        errorMessage = "Servidor não encontrado";
        break;
    case QNetworkReply::TimeoutError:
        errorMessage = "Timeout na conexão";
        break;
    case QNetworkReply::OperationCanceledError:
        errorMessage = "Operação cancelada";
        break;
    case QNetworkReply::SslHandshakeFailedError:
        errorMessage = "Falha na negociação SSL";
        break;
    case QNetworkReply::TemporaryNetworkFailureError:
        errorMessage = "Falha temporária de rede";
        break;
    case QNetworkReply::NetworkSessionFailedError:
        errorMessage = "Falha na sessão de rede";
        break;
    case QNetworkReply::BackgroundRequestNotAllowedError:
        errorMessage = "Requisição em background não permitida";
        break;
    case QNetworkReply::ProxyConnectionRefusedError:
        errorMessage = "Proxy recusou conexão";
        break;
    case QNetworkReply::ProxyNotFoundError:
        errorMessage = "Proxy não encontrado";
        break;
    default:
        errorMessage = QString("Erro de rede: %1").arg(static_cast<int>(error));
        break;
    }

    emit networkError(errorMessage);
}

// Métodos síncronos para compatibilidade
ServerStatus HttpClient::checkServerHealthSync()
{
    QEventLoop loop;
    ServerStatus result;

    connect(this, &HttpClient::healthCheckCompleted, &loop, [&](const ServerStatus& status) {
        result = status;
        loop.quit();
    });

    connect(this, &HttpClient::networkError, &loop, [&](const QString& error) {
        result.online = false;
        result.error_message = error;
        loop.quit();
    });

    checkServerHealth();
    loop.exec();

    return result;
}

ProcessingResult HttpClient::processTextSync(const QString& text)
{
    QEventLoop loop;
    ProcessingResult result;

    connect(this, &HttpClient::textProcessingCompleted, &loop, [&](const ProcessingResult& res) {
        result = res;
        loop.quit();
    });

    connect(this, &HttpClient::networkError, &loop, [&](const QString& error) {
        result.success = false;
        result.error_message = error;
        loop.quit();
    });

    processText(text);
    loop.exec();

    return result;
}