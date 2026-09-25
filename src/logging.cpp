#include "logging.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>

#include <cstdio>

namespace {
QFile *logFile = nullptr;
QMutex logMutex;

void fileMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &message) {
    const char *label = "INFO";
    switch (type) {
    case QtDebugMsg: label = "DEBUG"; break;
    case QtWarningMsg: label = "WARN"; break;
    case QtCriticalMsg: label = "ERROR"; break;
    case QtFatalMsg: label = "FATAL"; break;
    case QtInfoMsg: break;
    }
    const auto line = QString("%1 [%2] %3\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), label, message);
    QMutexLocker lock(&logMutex);
    if (logFile && logFile->isOpen()) {
        QTextStream(logFile) << line;
        logFile->flush();
    }
    fprintf(stderr, "%s", qPrintable(line));
}
}

QString logFilePath() {
    const auto directory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return QDir(directory).filePath("sony-headphones-linux.log");
}

void installFileLogger() {
    const auto directory = QFileInfo(logFilePath()).dir();
    directory.mkpath(".");
    logFile = new QFile(logFilePath());
    logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    qInstallMessageHandler(fileMessageHandler);
    qInfo().noquote() << "Sony Headphones started. Log file:" << logFilePath();
}
