#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QIODevice>

class FileProcessingException : public std::exception {
private:
    QString message;

public:
    explicit FileProcessingException(const QString& msg) : message(msg) {}
    const char* what() const noexcept override {
        static QByteArray bytes = message.toUtf8();
        return bytes.constData();
    }
    QString getMessage() const { return message; }
};

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);

    static QString readFile(const QString& filepath);
    static bool writeFile(const QString& filepath, const QString& content);
    static bool fileExists(const QString& filepath);
    static QString getFileExtension(const QString& filepath);
    static qint64 getFileSize(const QString& filepath);
    static QString getFileName(const QString& filepath);
    static QString getBaseName(const QString& filepath);

signals:
    void fileReadProgress(int percentage);
    void fileWriteProgress(int percentage);

public slots:

};

#endif // FILEPROCESSOR_H