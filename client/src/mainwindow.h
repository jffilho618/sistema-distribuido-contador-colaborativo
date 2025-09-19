#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QSplitter>
#include <QProgressBar>
#include <QTimer>
#include <QApplication>
#include <QFont>
#include <QFileInfo>
#include <QScrollBar>

#include "httpclient.h"
#include "fileprocessor.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void configureServer();
    void checkServerStatus();
    void processText();
    void processFile();
    void onHealthCheckCompleted(const ServerStatus& status);
    void onTextProcessingCompleted(const ProcessingResult& result);
    void onNetworkError(const QString& error);
    void updateProcessingProgress();

private:
    // UI Components
    void setupUI();
    void createServerConfigGroup();
    void createProcessingGroup();
    void createTextInputGroup();
    void createResultsGroup();
    void createStatusBar();
    void connectSignals();

    // UI Helper methods
    void updateStatus(const QString& message);
    void updateResults(const QString& results);
    void setButtonsEnabled(bool enabled);
    void showError(const QString& message);
    void showInfo(const QString& message);
    void showProcessingProgress(bool show);

    // Processing methods
    void processTextContent(const QString& text, const QString& source);
    QString formatResult(const ProcessingResult& result, const QString& source);
    QString selectFile();
    void clearTextInput();

    // UI Widgets - Server Config
    QGroupBox* serverConfigGroup;
    QLineEdit* hostLineEdit;
    QSpinBox* portSpinBox;
    QPushButton* configButton;
    QPushButton* statusButton;

    // UI Widgets - Processing
    QGroupBox* processingGroup;
    QPushButton* fileButton;
    QPushButton* textButton;

    // UI Widgets - Text Input
    QGroupBox* textInputGroup;
    QTextEdit* textInput;
    QPushButton* processTextAreaButton;

    // UI Widgets - Results
    QGroupBox* resultsGroup;
    QTextEdit* resultsDisplay;

    // UI Widgets - Status
    QProgressBar* progressBar;
    QLabel* statusLabel;
    QTimer* progressTimer;

    // Backend
    HttpClient* httpClient;

    // State
    bool isProcessing;
    QString lastUsedDirectory;
};

#endif // MAINWINDOW_H