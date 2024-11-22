#pragma once
#include <functional>
#include <iostream>
#include <Windows.h>

#include <QMessageBox>
#include <QString>
#include <QObject>
#include <QThread>
#include <QTextCodec>
#include <QTextDecoder>

class SfMWorker : public QObject {
    Q_OBJECT

public:
    SfMWorker(QObject* parent = nullptr) : QObject(parent) {}

    void setParameters(const QString& cameraIntrinsics, const QString& imageFolderPath, const QString& algorithm, const QString& saveFormat) {
        this->cameraIntrinsics = cameraIntrinsics;
        this->imageFolderPath = imageFolderPath;
        this->algorithm = algorithm;
        this->saveFormat = saveFormat;
    }

public slots:
    void process() {
        bool success = performSfMReconstruction(cameraIntrinsics, imageFolderPath, algorithm, saveFormat,
            [this](const QString& message) {
                emit logMessage(message);
            });

        if (success) {
            emit finished();
        }
        else {
            emit error("SFM重建失败");
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

    bool performSfMReconstruction(const QString& cameraIntrinsics,
        const QString& imageFolderPath,
        const QString& algorithm,
        const QString& save,
        const std::function<void(const QString&)>& logCallback);
};

