#pragma once
#include <functional>
#include <iostream>
#include <Windows.h>

#include <QMessageBox>
#include <QString>
#include <QObject>
#include <QThread>

class StreamBuffer : public std::streambuf {
public:
    std::function<void(const std::string&)> outputCallback;

protected:
    virtual int_type overflow(int_type v) {
        char c = static_cast<char>(v);
        if (outputCallback) outputCallback(std::string(1, c));
        return v;
    }

    virtual std::streamsize xsputn(const char* p, std::streamsize n) {
        if (outputCallback) outputCallback(std::string(p, n));
        return n;
    }
};

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

