#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QUrl>

struct ProcessingResult {
    bool success;
    int letters_count;
    int numbers_count;
    int total_characters;
    double processing_time_ms;
    QString error_message;
};

struct ServerStatus {
    bool online;
    QString status;
    QString timestamp;
    QString error_message;
};

class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);

    void setServer(const QString& host, int port);
    void setTimeout(int seconds);
    QString getServerUrl() const;

    // Métodos assíncronos
    void checkServerHealth();
    void processText(const QString& text);

    // Métodos síncronos (para compatibilidade)
    ServerStatus checkServerHealthSync();
    ProcessingResult processTextSync(const QString& text);

private slots:
    void onHealthCheckFinished();
    void onProcessTextFinished();
    void onRequestTimeout();

signals:
    void healthCheckCompleted(const ServerStatus& status);
    void textProcessingCompleted(const ProcessingResult& result);
    void networkError(const QString& error);

private:
    QNetworkAccessManager* networkManager;
    QString serverHost;
    int serverPort;
    int timeoutSeconds;
    QTimer* timeoutTimer;
    QElapsedTimer requestTimer;
    QNetworkReply* currentReply;

    QNetworkRequest createRequest(const QString& endpoint);
    ServerStatus parseHealthResponse(const QByteArray& data);
    ProcessingResult parseProcessResponse(const QByteArray& data);
    void handleNetworkError(QNetworkReply::NetworkError error);
};

#endif // HTTPCLIENT_H