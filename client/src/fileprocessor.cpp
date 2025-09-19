#include "fileprocessor.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QDebug>

FileProcessor::FileProcessor(QObject *parent)
    : QObject(parent)
{
}

QString FileProcessor::readFile(const QString& filepath)
{
    QFile file(filepath);

    if (!file.exists()) {
        throw FileProcessingException(QString("Arquivo não encontrado: %1").arg(filepath));
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileProcessingException(QString("Não foi possível abrir o arquivo: %1").arg(filepath));
    }

    QTextStream stream(&file);
    // Qt6 changed encoding API
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        stream.setEncoding(QStringConverter::Utf8);
    #else
        stream.setCodec("UTF-8");
    #endif

    QString content = stream.readAll();
    file.close();

    return content;
}

bool FileProcessor::writeFile(const QString& filepath, const QString& content)
{
    try {
        QFile file(filepath);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream stream(&file);
        // Qt6 changed encoding API
        #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            stream.setEncoding(QStringConverter::Utf8);
        #else
            stream.setCodec("UTF-8");
        #endif
        stream << content;

        file.close();
        return true;

    } catch (const std::exception&) {
        return false;
    }
}

bool FileProcessor::fileExists(const QString& filepath)
{
    return QFile::exists(filepath);
}

QString FileProcessor::getFileExtension(const QString& filepath)
{
    QFileInfo fileInfo(filepath);
    return fileInfo.suffix();
}

qint64 FileProcessor::getFileSize(const QString& filepath)
{
    if (!fileExists(filepath)) {
        return 0;
    }

    QFileInfo fileInfo(filepath);
    return fileInfo.size();
}

QString FileProcessor::getFileName(const QString& filepath)
{
    QFileInfo fileInfo(filepath);
    return fileInfo.fileName();
}

QString FileProcessor::getBaseName(const QString& filepath)
{
    QFileInfo fileInfo(filepath);
    return fileInfo.baseName();
}