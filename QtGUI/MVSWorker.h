#pragma once
#include <functional>
#include <iostream>
#include <Windows.h>

#include <QMessageBox>
#include <QString>
#include <QObject>
#include <QThread>

class MVSWorker : public QObject {
    Q_OBJECT

public:
    MVSWorker(QObject* parent = nullptr) : QObject(parent) {}

    void setParameters(const QString& cameraIntrinsics, const QString& imageFolderPath, const QString& algorithm, const QString& saveFormat) {
        this->cameraIntrinsics = cameraIntrinsics;
        this->imageFolderPath = imageFolderPath;
        this->algorithm = algorithm;
        this->saveFormat = saveFormat;
    }

public slots:
    void process() {
        bool success = performMVSReconstruction(cameraIntrinsics, imageFolderPath, algorithm, saveFormat,
            [this](const QString& message) {
                emit logMessage(message);
            });

        if (success) {
            emit finished();
        }
        else {
            emit error("MVS 失败");
        }
    }

signals:
    void logMessage(const QString& message);
    void finished();
    void error(const QString& errorMessage);

private:
    QString cameraIntrinsics;
    QString imageFolderPath;
    QString algorithm;
    QString saveFormat;

    bool performMVSReconstruction(const QString& cameraIntrinsics,
        const QString& imageFolderPath,
        const QString& algorithm,
        const QString& save,
        const std::function<void(const QString&)>& logCallback);
};

