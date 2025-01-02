#pragma once
#include <functional>
#include <iostream>
#include <filesystem>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <Windows.h>

#include <QMessageBox>
#include <QString>
#include <QObject>
#include <QThread>
#include <QTextCodec>
#include <QTextDecoder>

class ImageWorker : public QObject {
    Q_OBJECT

public:
    ImageWorker(QObject* parent = nullptr) : QObject(parent) {}

    void setParameters(const QString& imageFolderPath) {
        this->imageFolderPath = imageFolderPath;
    }

public slots:
    void process() {
        bool success = performImageGet(imageFolderPath,
            [this](const QString& message) {
                emit logMessage(message);
            });

        if (success) {
            emit finished();
        }
        else {
            emit error("图像获取重建失败");
        }
    }

signals:
    void logMessage(const QString& message);
    void finished();
    void error(const QString& errorMessage);

private:
    QString imageFolderPath;

    bool performImageGet(
        const QString& imageFolderPath,
        const std::function<void(const QString&)>& logCallback);
};

